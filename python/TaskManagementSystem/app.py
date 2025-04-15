from flask import Flask, request, jsonify, send_from_directory
import json
import os
from datetime import datetime

app = Flask(__name__)

# 数据存储路径
TASK_FILE = "task.json"
IMAGE_DIR = "image"

# 初始化数据文件和图片目录
if not os.path.exists(TASK_FILE):
  with open(TASK_FILE, "w") as f:
    json.dump([], f)
if not os.path.exists(IMAGE_DIR):
  os.makedirs(IMAGE_DIR)

# 统一JSON响应格式
def response(success, data=None, message=""):
  return jsonify({"success": success, "data": data, "message": message})

# 获取所有任务
@app.route("/tasks", methods=["GET"])
def get_tasks():
  try:
    with open(TASK_FILE, "r") as f:
      tasks = json.load(f)
    return response(True, tasks)
  except Exception as e:
    return response(False, message=str(e))

# 获取单个任务详情
@app.route("/task/<int:task_id>", methods=["GET", "DELETE", "PUT"])
def get_or_delete_task(task_id):
  try:
    with open(TASK_FILE, "r") as f:
      tasks = json.load(f)
      task = next((t for t in tasks if t["id"] == task_id), None)
      if task:
        if request.method == "GET":
          return response(True, task)  # 返回单个任务的详细信息
        elif request.method == "DELETE":
          tasks = [t for t in tasks if t["id"] != task_id]
          with open(TASK_FILE, "w") as f:
            json.dump(tasks, f, indent=2)
          return response(True, message="任务已删除")
        elif request.method == "PUT":
          # 新增: 处理 PUT 请求，更新任务
          task_data = request.json
          for t in tasks:
            if t["id"] == task_id:
              t.update(task_data)
              break
          with open(TASK_FILE, "w") as f:
            json.dump(tasks, f, indent=2)
          return response(True, task_data)
      else:
        return response(False, message="任务不存在")
  except Exception as e:
    return response(False, message=str(e))

# 创建/更新任务
@app.route("/task", methods=["POST", "PUT"])
def manage_task():
  try:
    task_data = request.json
    with open(TASK_FILE, "r+") as f:
      tasks = json.load(f)
      if request.method == "POST":
        task_data["id"] = len(tasks) + 1
        task_data["time"] = datetime.now().strftime("%Y-%m-%d")  # 新增: 使用当前日期作为任务时间
        tasks.append(task_data)
      elif request.method == "PUT":
        for task in tasks:
          if task["id"] == task_data["id"]:
            task.update(task_data)
            break
      f.seek(0)
      json.dump(tasks, f, indent=2)
      f.truncate()
    return response(True, task_data)
  except Exception as e:
    return response(False, message=str(e))

# 编辑任务
@app.route("/task/edit/<int:task_id>", methods=["PUT"])
def edit_task(task_id):
  try:
    task_data = request.json
    with open(TASK_FILE, "r+") as f:
      tasks = json.load(f)
      for task in tasks:
        if task["id"] == task_id:
          task.update(task_data)
          break
      f.seek(0)
      json.dump(tasks, f, indent=2)
      f.truncate()
    return response(True, task_data)  # 修改: 返回完整的任务数据
  except Exception as e:
    return response(False, message=str(e))

# 删除任务
@app.route("/task/<int:task_id>", methods=["DELETE"])
def delete_task(task_id):
  try:
    with open(TASK_FILE, "r+") as f:
      tasks = json.load(f)
      tasks = [task for task in tasks if task["id"] != task_id]
      f.seek(0)
      json.dump(tasks, f, indent=4)
      f.truncate()
    return response(True, message="任务已删除")
  except Exception as e:
    return response(False, message=str(e))

# 标记任务完成或取消完成
@app.route("/task/complete/<int:task_id>", methods=["POST"])
def complete_task(task_id):
  try:
    # 新增: 获取请求中的 completed 状态
    data = request.json
    completed = data.get("completed", False)  # 默认为 False（未完成）

    with open(TASK_FILE, "r+") as f:
      tasks = json.load(f)
      for task in tasks:
        if task["id"] == task_id:
          task["completed"] = completed  # 根据传入状态更新任务完成状态
          break
      f.seek(0)
      json.dump(tasks, f, indent=2)
      f.truncate()
    return response(True, message=f"任务已{'完成' if completed else '取消完成'}")
  except Exception as e:
    return response(False, message=str(e))

# 图片上传
@app.route("/image", methods=["POST"])
def upload_image():
  if "image" not in request.files:
    return response(False, message="没有文件部分")
  file = request.files["image"]
  if file.filename == "":
    return response(False, message="未选择文件")
  filepath = os.path.join(IMAGE_DIR, file.filename)
  file.save(filepath)
  return response(True, {
    "filename": file.filename
    })

# 获取图片
@app.route("/image/<filename>", methods=["GET"])
def get_image(filename):
  return send_from_directory(IMAGE_DIR, filename)

# 新增静态文件支持
@app.route("/", methods=["GET"])
def serve_index():
  return send_from_directory(".", "index.html")

if __name__ == "__main__":
  app.run(debug=True)