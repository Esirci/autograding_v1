import subprocess

def run_job(cmd):
    ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return ret.stdout.decode(('UTF-8'))
    
if __name__ == '__main__':
    # Compile
    #run_job("gcc main.c")
    run_job("gcc -Wall -o hw2 hw2.c")

    # Run
    #msg = run_job("./a.out").rstrip()
    msg = run_job("./hw2").rstrip()
    if msg == "Hello World!":
        print("Ok! expected output is corerect.")
    else:
        print("Error. The correct answer is `Hello World!`, but your output is:")
        print("```")
        print(msg)
        print("```")
