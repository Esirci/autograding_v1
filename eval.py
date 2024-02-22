import subprocess

def run_job(cmd):
    ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return ret.stdout.decode(('UTF-8'))



def test_shell_commands():
    # C programını derleme
    compile_process = subprocess.run(["gcc", "-Wall", "hw2.c", "-o", "shell"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    if compile_process.returncode != 0:
        print("Hata: Kod derlenemedi.")
        print("Hata çıktısı:")
        print(compile_process.stderr)
        return
    
    # Test edilecek komutlar ve argümanları
    test_commands = [
        #"ls > cmd1.txt",
        "echo "aaa,bb,cc,dd" | grep "aa"",
        "q"
        #"grep "portakal" searchFile.txt",
        #"cat file.txt",
        #"sed 's/apple/orange/g' searchFile.txt > cmd2.txt"
    ]

    for command in test_commands:
        # C programını çalıştırma
        execute_process = subprocess.run(["./shell"], input=command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True, shell=True)
        
        if execute_process.returncode == 0:
            print(f"{command} komutu başarıyla çalıştırıldı.")
            print("Çıktı:")
            print(execute_process.stdout)
        else:
            print(f"{command} komutu çalıştırılamadı.")
            print("Hata çıktısı:")
            print(execute_process.stderr)
    


# Test fonksiyonunu çağırma
test_shell_commands()


