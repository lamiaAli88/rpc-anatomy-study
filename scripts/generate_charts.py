import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# قراءة البيانات
in_proc = pd.read_csv('data/in_process_results.csv')
unix_sock = pd.read_csv('data/unix_socket_results.csv')
tcp_loop = pd.read_csv('data/tcp_loopback_results.csv')

# حساب الوسيط (Median) بالنص ثانية وتحويله إلى الميكرو ثانية (us)
summary = {
    'In-Process': {
        'Marshal/Unmarshal': np.median(in_proc['pb_serialize_ns'] + in_proc['pb_deserialize_ns']) / 1000.0,
        'Syscall/Kernel/Wire': np.median(in_proc['direct_ns']) / 1000.0
    },
    'Unix Socket': {
        'Marshal/Unmarshal': np.median(unix_sock['pb_serialize_ns'] + unix_sock['pb_deserialize_ns']) / 1000.0,
        'Syscall/Kernel/Wire': (np.median(unix_sock['total_rtt_ns']) - np.median(unix_sock['pb_serialize_ns'] + unix_sock['pb_deserialize_ns'])) / 1000.0
    },
    'TCP Loopback': {
        'Marshal/Unmarshal': np.median(tcp_loop['pb_serialize_ns'] + tcp_loop['pb_deserialize_ns']) / 1000.0,
        'Syscall/Kernel/Wire': (np.median(tcp_loop['total_rtt_ns']) - np.median(tcp_loop['pb_serialize_ns'] + tcp_loop['pb_deserialize_ns'])) / 1000.0
    }
}

df = pd.DataFrame(summary).T

# رسم الـ Stacked Bar Chart
fig, ax = plt.subplots(figsize=(8, 6))
df.plot(kind='bar', stacked=True, ax=ax, colormap='viridis')

plt.title('RPC Latency Breakdown (The Anatomy of a Millisecond)', fontsize=12, fontweight='bold')
plt.ylabel('Time (Microseconds - µs)')
plt.xlabel('Transport Mechanism')
plt.xticks(rotation=0)
plt.grid(axis='y', linestyle='--', alpha=0.7)
plt.legend(title='RPC Layer/Component')

plt.tight_layout()
plt.savefig('plots/rpc_breakdown_stacked_chart.png', dpi=300)
print("Stacked Bar Chart generated successfully and saved to plots/rpc_breakdown_stacked_chart.png!")
