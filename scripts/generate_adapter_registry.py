#!/usr/bin/env python3
"""
从 pipeline.yaml 自动生成 adapter_registry.def
这样只需维护一个配置文件，自动生成所有TypeAdapter注册代码
"""

import yaml
import os
from pathlib import Path

def generate_adapter_registry():
    project_root = Path(__file__).parent.parent
    pipeline_config = project_root / "modules" / "common" / "config" / "pipeline.yaml"
    output_file = project_root / "modules" / "common" / "include" / "common" / "adapter_registry.def"
    
    # 读取pipeline配置
    with open(pipeline_config, 'r') as f:
        config = yaml.safe_load(f)
    
    if 'data_keys' not in config:
        print("❌ No data_keys found in pipeline.yaml")
        return
    
    print("🚀 从 pipeline.yaml 生成 adapter_registry.def...")
    
    # 生成注册列表内容
    lines = [
        "// TypeAdapter注册列表 - 自动生成",
        "// 源文件: modules/common/config/pipeline.yaml", 
        "// 生成工具: scripts/generate_adapter_registry.py",
        "// 语法：ADAPTER(KEY, PROTO_TYPE, ROS_TYPE)",
        "",
    ]
    
    for key, data_config in config['data_keys'].items():
        # 从 proto_type 推断 key (去掉命名空间)
        proto_parts = data_config['proto_type'].split('::')
        if len(proto_parts) >= 3:
            # "pilot_msgs::localization::ImuData" → "imu" 
            type_name = proto_parts[-1].lower()  # "ImuData" → "imudata"
            if type_name.endswith('data'):
                simple_key = type_name[:-4]  # "imudata" → "imu"
            else:
                simple_key = type_name
        else:
            simple_key = key
        
        line = f"ADAPTER({simple_key}, {data_config['proto_type']}, {data_config['ros_type']})"
        lines.append(line)
    
    lines.append("")
    lines.append("// 未来添加新数据类型时，在pipeline.yaml的data_keys中配置即可")
    
    # 写入文件
    with open(output_file, 'w') as f:
        f.write('\n'.join(lines) + '\n')
    
    print(f"✅ 生成完成: {output_file}")
    print(f"📝 包含 {len(config['data_keys'])} 个TypeAdapter注册")
    
    for key, data_config in config['data_keys'].items():
        print(f"   - {key}: {data_config['proto_type']} ↔ {data_config['ros_type']}")

if __name__ == '__main__':
    generate_adapter_registry()
