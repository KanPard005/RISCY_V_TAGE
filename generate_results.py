import os
import numpy as np
import pandas as pd
import re

rows = sorted([f[:10] for f in os.listdir("hashed_perceptron")])
columns = ["ltage","tage","hashed_perceptron"]

df_mpki = pd.DataFrame(np.random.randn(len(rows),len(columns)), index=rows, columns=columns)
df_accuracy = pd.DataFrame(np.random.randn(len(rows),len(columns)), index=rows, columns=columns)

for folder in columns:
    for file in os.listdir(folder):
        print(folder, file)
        f = open(os.path.join(folder,file),'r')
        file_name = file[:10]
        text = f.read()
        df_mpki.loc[file_name,folder] = float(re.compile(r'MPKI: \d+\.?\d*').search(text).group().split()[-1])
        df_accuracy.loc[file_name,folder] = float(re.compile("Branch Prediction Accuracy: \d+\.?\d*").search(text).group().split()[-1])

df_mpki.to_csv("mpki.csv")
df_accuracy.to_csv("accuracy.csv")

print(df_mpki)
print(df_accuracy)