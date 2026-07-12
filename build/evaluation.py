import os
import subprocess
import time 
import statistics
import cpuinfo 
import json

data_json = {}
evaluation_label = ""

def evaluate():
    print(f"label: {evaluation_label}")
    with open('../output/evaluation.txt', 'r') as f:
        lines = f.readlines()
        accuracy = [float(line.split()[0]) for line in lines]
        execution_time = [float(line.split()[1]) for line in lines]

        accuracy_mean = statistics.fmean(accuracy)
        execution_time_mean = statistics.fmean(execution_time)
        min_accuracy = min(accuracy)

        data_json["accuracy_values"] = accuracy
        data_json["execution_time"] = execution_time 

    with open('../output/evaluation_data_' + evaluation_label + '.json', 'w', encoding='utf-8') as f:
        json.dump(data_json, f, ensure_ascii=False, indent=4)
    
    subprocess.run(["rm", "../output/evaluation.txt"]) 
    #print(f"Data: {json.dumps(data_json, indent=4)}\n") 
    
    print(f"\nDurchschnittliche Genauigkeit: {accuracy_mean}\nDurchschnittliche Ausführungszeit: {execution_time_mean}s\nMinimale Genauigkeit: {min_accuracy}\n")

def main():
    cpu_core = 25
    global evaluation_label 
    evaluation_label = input("Label für die Messung: ")
    data_json["cpu"] = cpuinfo.get_cpu_info()

    for i in range(10): # execute tests for the covert channel
        subprocess.run(["taskset", "-c", f"{cpu_core}", "./spec_exec_leak"])
        with open('../output/array_timing.txt', 'r') as f:
            lines = f.readlines() 
            timings = [int(line.split()[0]) for line in lines]
            data_json[f"spec_exec_{i}"] = timings

    for i in range(100): # execute attacker 100 times
        subprocess.run(["taskset", "-c", f"{cpu_core}", "./spectre_attacker"])
        time.sleep(0.1)
    evaluate() 

if __name__ == "__main__":
    main()