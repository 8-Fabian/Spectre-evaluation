import os
import shutil
import json 
import matplotlib.pyplot as plt
from contextlib import chdir
import pandas 

def clean_directories():

    # clean
    for filename in os.listdir("../output_json"):
        with open(os.path.join("../output_json", filename), 'r') as f: 
            eval_data = json.load(f) 
            if os.path.exists(f"./{eval_data["cpu"]["brand_raw"]}"):
                shutil.rmtree(f"./{eval_data["cpu"]["brand_raw"]}")
            os.makedirs(f"./{eval_data["cpu"]["brand_raw"]}")

    # create folders
    for filename in os.listdir("../output_json"):
        with open(os.path.join("../output_json", filename), 'r') as f: 
            label = f.name[31:]     # remove first part of filename 
            label = label.removesuffix(".json") # remove json ending to get label
            
            eval_data = json.load(f)

            with chdir(f"./{eval_data["cpu"]["brand_raw"]}"):
                os.makedirs(f"./{label}")

def statistics_stuff():
    ### Accuracy Boxplot
    acc_data = []
    x_labels = []
    for filename in os.listdir("../output_json"):
        with open(os.path.join("../output_json", filename), 'r') as f: 
            label = f.name[31:]     # remove first part of filename
            label = label.removesuffix(".json") # remove json ending to get label
            
            eval_data = json.load(f) 
        
            with chdir(f"./{eval_data["cpu"]["brand_raw"]}"):
                with chdir(f"./{label}"):
                    specv1_acc = eval_data["v1_isolated"]["accuracy"]
                    acc_data.append(specv1_acc)
                    x_labels.append(eval_data["cpu"]["brand_raw"])

    plt.boxplot(acc_data, tick_labels=x_labels)
    plt.ylabel("Genauigkeit (%)")
    plt.title("Genauigkeit")
    plt.savefig(f"acc_boxplot.png")
    plt.cla()

    for filename in os.listdir("../output_json"):
        with open(os.path.join("../output_json", filename), 'r') as f: 
            label = f.name[31:]     # remove first part of filename
            label = label.removesuffix(".json") # remove json ending to get label
            
            eval_data = json.load(f) 
        
            with chdir(f"./{eval_data["cpu"]["brand_raw"]}"):
                with chdir(f"./{label}"):
                    ## NUMBERS
                    #specv1_acc = pandas.Series(eval_data["v1_isolated"]["accuracy"])
                    #print(specv1_acc.describe())
                    #with open("./statistics.txt", 'a') as statistics_file:
                    #    f.write(f"{specv1_acc.describe()}")

                    ## GRAPHICS

                    ###
                    for i in range(10):
                        spec_exec_data = eval_data[f"spec_exec_isolated_{i}"]
                        #print(f"Spec exec data {i}: {spec_exec_data}")
                        plt.plot(range(256), spec_exec_data)
                        plt.xlabel("Index im Array")
                        plt.ylabel("Speicherladezeit (Taktzyklen)")
                        plt.savefig(f"spec_exec_isolated_{i}.png")
                        plt.cla()

                    ### Accuracy Boxplot
                    specv1_acc = eval_data["v1_isolated"]["accuracy"]
                    plt.boxplot(specv1_acc)
                    plt.ylabel("Genauigkeit (%)")
                    
                    #plt.title("Genauigkeit")
                    plt.savefig(f"acc_boxplot.png")
                    plt.cla()

def main():
    clean_directories()
    statistics_stuff()

    return

if __name__ == "__main__":
    main()