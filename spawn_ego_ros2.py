import carla, random

client = carla.Client('127.0.0.1', 2000)
client.set_timeout(5.0)
world = client.get_world()
bp = world.get_blueprint_library()

# 1) 选一个你机器里确实存在的车型（你刚打印有这个）
veh_bp = bp.find('vehicle.taxi.ford')

# 车辆也支持 ros_name；设为 ego 后会创建 /carla/ego/vehicle_control_cmd 订阅
if any(a.id == 'ros_name' for a in veh_bp):
    veh_bp.set_attribute('ros_name', 'ego')

# 2) 随机打乱 spawn 点，逐个试；失败就换下一个（不会抛异常）
spawn_points = world.get_map().get_spawn_points()
random.shuffle(spawn_points)

ego = None
for sp in spawn_points:
    actor = world.try_spawn_actor(veh_bp, sp)
    if actor is not None:
        ego = actor
        break

if ego is None:
    raise RuntimeError("没有空闲出生点：请先清掉已有车辆或换张地图再试。")

print("Spawned ego:", ego.id)

# 3) 挂 IMU/GNSS 并启用 ROS 话题发布
def attach_sensor(type_id, ros_name, tf):
    s_bp = bp.find(type_id)
    if any(a.id == 'ros_name' for a in s_bp):
        s_bp.set_attribute('ros_name', ros_name)
    s = world.spawn_actor(s_bp, tf, attach_to=ego)
    s.enable_for_ros()   # 关键：让这个传感器开始对 ROS2 发布
    return s

imu  = attach_sensor('sensor.other.imu',  'imu',  carla.Transform())
gnss = attach_sensor('sensor.other.gnss', 'gnss', carla.Transform())

print("Ego + IMU + GNSS ready. 现在去 ROS2 看话题。")
