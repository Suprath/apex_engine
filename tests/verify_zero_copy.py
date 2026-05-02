import numpy as np
import apex_python
import time
import sys

def verify_zero_copy():
    print("=== Zero-Copy Handoff Verification ===")
    
    # 1. Allocate 100MB of data (12.5M uint64 elements)
    # Stride of 8 bytes
    num_elements = 12_500_000 
    data = np.random.get_state()[1][:num_elements].view(np.uint64)
    if len(data) < num_elements:
        data = np.zeros(num_elements, dtype=np.uint64)
    
    print(f"Allocated {data.nbytes / (1024*1024):.2f} MB NumPy array")

    # 2. Setup Apex Engine
    engine = apex_python.ApexEngine()
    
    # Simple schema: one 64-bit field
    fields = [
        {"name": "f0", "offset": 0, "bit_width": 64, "is_signed": False}
    ]
    engine.register_schema("ZeroCopyTest", fields, 8)
    
    # Constant logic: f0 > 0
    logic = apex_python.create_universal_test_logic()
    engine.set_logic("ZeroCopyTest", logic, mode="BIT_SLICED")

    # 3. Measure Handoff Time
    # We measure ONLY the call to execute. If it's > 10ms, it's definitely copying.
    start_ns = time.perf_counter_ns()
    matches = engine.execute("ZeroCopyTest", data)
    end_ns = time.perf_counter_ns()
    
    duration_ms = (end_ns - start_ns) / 1_000_000
    print(f"Execution + Handoff Time: {duration_ms:.4f} ms")
    print(f"Matches found: {matches}")

    # Threshold: On M3 Air, a 100MB copy takes ~15-20ms. 
    # If we are under 5ms (including actual execution), we are definitely zero-copy.
    if duration_ms < 5.0:
        print("[PASS] Zero-copy handoff confirmed (Sub-5ms handoff for 100MB)")
    else:
        print("[FAIL] High latency detected. Potential data copy in binding layer.")

if __name__ == "__main__":
    verify_zero_copy()
