import os
import subprocess
import time 
import statistics
import cpuinfo 
import json
import shutil 

###############################################################
###                                                         ###
###                 Evaluation:                             ###
### 1. covert channel tests auf isoliertem CPU Kern         ###
### 2. spectre v1 auf isoliertem CPU Kern                   ###
### 3. spectre v2 auf isoliertem CPU Kern                   ###
### 4. spectre v1 mit Mitigation auf isoliertem CPU Kern    ###
### 5. spectre v2 mit Mitigation auf isoliertem CPU Kern    ###
### 6. covert channel tests mit cache stress                ###
### 7. spectre v1 mit Cache stress                          ###
### 8. spectre v2 mit Cache stress                          ###
### 9. spectre v1 mit Mitigation und Cache stress           ###
### 10. spectre v2 mit Mitigation und Cache stress          ###
###                                                         ###
###############################################################

data_json = {}
evaluation_label = ""
cpu_core = 0

def test_covert_channel(context:str):
    for i in range(10): # execute tests for the covert channel
        subprocess.run(["taskset", "-c", f"{cpu_core}", "./spec_exec_leak"])
        with open('../output/array_timing.txt', 'r') as f:
            lines = f.readlines() 
            timings = [int(line.split()[0]) for line in lines]
            data_json[f"spec_exec_{context}_{i}"] = timings 
    os.remove("../output/array_timing.txt") 

def test_spectre_v1(mitigated:bool, context:str, iterations:int=50):   # Mitigated or not / Isolated or run with noise / number of test iterations
    if mitigated == True:
        victim_process = subprocess.Popen(["taskset", "-c", f"{cpu_core}", "./spectre_mitigated"])
        # execute attacker 
        for i in range(iterations): 
            subprocess.run(["taskset", "-c", f"{cpu_core}", "./spectre_attacker"]) 
            time.sleep(0.1) 
        victim_process.terminate() 
        victim_process.wait()
        # Process data
        with open('../output/evaluation.txt', 'r') as f:
            lines = f.readlines()
            accuracy = [float(line.split()[0]) for line in lines]
            execution_time = [float(line.split()[1]) for line in lines]
            average_cycles = [round(float(line.split()[2])) for line in lines]

            data_json[f"v1_mitigated_{context}"] = {}
            data_json[f"v1_mitigated_{context}"]["accuracy"] = accuracy
            data_json[f"v1_mitigated_{context}"]["time"] = execution_time 
            data_json[f"v1_mitigated_{context}"]["flush_reload_cycles"] = average_cycles 
        with open('../output/victim_output.txt', 'r') as f:
            lines = f.readlines()
            cycles_taken = [float(line.split()[0]) for line in lines]
            data_json[f"v1_mitigated_{context}"]["victim_cycles"] = cycles_taken 
        os.remove("../output/evaluation.txt") 
        os.remove("../output/victim_output.txt")
    else:
        victim_process = subprocess.Popen(["taskset", "-c", f"{cpu_core}", "./spectre_victim"])
        # execute attacker 
        for i in range(iterations): 
            subprocess.run(["taskset", "-c", f"{cpu_core}", "./spectre_attacker"]) 
            time.sleep(0.1)
        victim_process.terminate() 
        victim_process.wait()
        # Process data 
        with open('../output/evaluation.txt', 'r') as f:
            lines = f.readlines()
            accuracy = [float(line.split()[0]) for line in lines]
            execution_time = [float(line.split()[1]) for line in lines]
            average_cycles = [round(float(line.split()[2])) for line in lines]

            data_json[f"v1_{context}"] = {}
            data_json[f"v1_{context}"]["accuracy"] = accuracy
            data_json[f"v1_{context}"]["time"] = execution_time 
            data_json[f"v1_{context}"]["avg_cycles"] = average_cycles 
        with open('../output/victim_output.txt', 'r') as f:
            lines = f.readlines()
            cycles_taken = [float(line.split()[0]) for line in lines]
            data_json[f"v1_{context}"]["victim_cycles"] = cycles_taken 
        os.remove("../output/evaluation.txt") 
        os.remove("../output/victim_output.txt")

def test_spectre_v2(mitigated:bool, context:str, iterations:int=50):   # Mitigated or not / Isolated or run with noise / number of test iterations
    if mitigated == True:
        for i in range(iterations): # execute poc 
            subprocess.run(["taskset", "-c", f"{cpu_core}", "./spectre_v2_mitigated"])
            time.sleep(0.1)
        with open('../output/evaluation.txt', 'r') as f:
            lines = f.readlines()
            accuracy = [float(line.split()[0]) for line in lines]
            execution_time = [float(line.split()[1]) for line in lines]
            average_cycles = [round(float(line.split()[2])) for line in lines]

            data_json[f"v2_mitigated_{context}"] = {}
            data_json[f"v2_mitigated_{context}"]["accuracy"] = accuracy
            data_json[f"v2_mitigated_{context}"]["time"] = execution_time 
            data_json[f"v2_mitigated_{context}"]["avg_cycles"] = average_cycles 
        os.remove("../output/evaluation.txt") 
    else:
        for i in range(iterations): # execute poc 
            subprocess.run(["taskset", "-c", f"{cpu_core}", "./spectre_v2_poc"])
            time.sleep(0.1)
        with open('../output/evaluation.txt', 'r') as f:
            lines = f.readlines()
            accuracy = [float(line.split()[0]) for line in lines]
            execution_time = [float(line.split()[1]) for line in lines]
            average_cycles = [round(float(line.split()[2])) for line in lines]

            data_json[f"v2_{context}"] = {}
            data_json[f"v2_{context}"]["accuracy"] = accuracy
            data_json[f"v2_{context}"]["time"] = execution_time 
            data_json[f"v2_{context}"]["avg_cycles"] = average_cycles 
        os.remove("../output/evaluation.txt") 

def main():
    global evaluation_label 
    global cpu_core
    evaluation_label = input("Label für die Messung: ")
    cpu_core = input("CPU Kern auf dem getestet wird: ")

    data_json["cpu"] = cpuinfo.get_cpu_info()
    # Prepare output and make executables
    if os.path.exists("../output"):
        shutil.rmtree("../output")
    os.makedirs("../output")
    if not os.path.exists("../output_json"):
        os.makedirs("../output_json")

    subprocess.run(["cmake", "-S", "..", "-B", "../build", "-D" "BYTES_TO_BE_READ=10000"])
    subprocess.run(["cmake", "--build", "../build"])

    print("\nStarte Tests...")
    # 1.
    test_covert_channel("isolated") 
    
    # 2.
    print("\nSpectre v1 Tests...")
    test_spectre_v1(False, "isolated")  # not mitigated but isolated

    # 3. 
    print("\nSpectre v2 Tests...")
    test_spectre_v2(False, "isolated")  # not mitigated but isolated

    # 4. 
    print("\nMitigated Spectre v1 Tests...")
    test_spectre_v1(True, "isolated")  # mitigated and isolated

    # 5.
    print("\nMitigated Spectre v2 Tests...")
    test_spectre_v2(True, "isolated")  # mitigated and isolated

    # recompile 
    subprocess.run(["cmake", "-S", "..", "-B", "../build", "-D" "BYTES_TO_BE_READ=1000"])
    subprocess.run(["cmake", "--build", "../build"])

    # Start cache stress
    cache_stress = subprocess.Popen(["stress-ng", "--cache", "1", "--taskset", f"{cpu_core}"])
    print("\nStarting Tests with Cache Stress. This can take a few minutes...")
    # 6.
    test_covert_channel("noise") 
    
    # 7.
    print("\nSpectre v1 Tests...")
    test_spectre_v1(False, "noise", 5)  # not mitigated with noise

    # 8.
    print("\nSpectre v2 Tests...")
    test_spectre_v2(False, "noise")  # not mitigated with noise

    # 9. 
    print("\nMitigated Spectre v1 Tests...")
    test_spectre_v1(True, "noise", 5)  # mitigated and noise

    # 10.
    print("\nMitigated Spectre v2 Tests...")
    test_spectre_v2(True, "noise")  # mitigated and noise

    cache_stress.kill()
    with open('../output_json/evaluation_data_' + evaluation_label + '.json', 'w', encoding='utf-8') as f:
        json.dump(data_json, f, ensure_ascii=False, indent=4)
    

if __name__ == "__main__":
    main()