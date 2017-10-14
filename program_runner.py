import subprocess

f = open('data.txt', 'w')
i = 2

p = subprocess.Popen(["./virtmem", "100", str(i), "rand", "sort"],
                 stdout=subprocess.PIPE)

for ln in p.stdout:    
    f.write(ln)

f.close()
