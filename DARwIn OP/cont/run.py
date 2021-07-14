import os   
import fileinput
import shutil
import pathlib

dir=r"C:\Users\ANGEL\Documents\Verano 2021\Soccer\controllers\rotar3"
base=str(pathlib.Path(__file__).parent.resolve())+r"\controladorBASE"
nombre=dir.split("\\")[-1]
for file in os.listdir(base):
    if file.split(".")[0]==r"{nombre}":
        shutil.copyfile(base+"/"+file,dir+"/"+nombre+'.'+file.split(".")[-1])
    else:
        shutil.copy2(base+"/"+file,dir)

for f in os.listdir(dir):
    with fileinput.FileInput(dir+"/"+f, inplace=True) as file:
        for line in file:
            print(line.replace(r"{nombre}", nombre).replace(r"{nombre2}",nombre.upper()), end='')