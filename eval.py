import subprocess

def run_job(cmd):
    ret = subprocess.run(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    return ret.stdout.decode(('UTF-8'))



def run_tests():
    # main.c dosyasını derleme
    compile_process = subprocess.run(["gcc", "hw2.c", "-o", "hw2"], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    if compile_process.returncode != 0:
        print("Hata: Kod derlenemedi.")
        print("Hata çıktısı:")
        print(compile_process.stderr)
        return
    
    # Derlenen programı çalıştırma
    # Kullanıcıdan girdileri alarak çalıştırma
    user_input = input("Programa girmek istediğiniz veriyi girin: ")
    execute_process = subprocess.run(["./hw2", user_input], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    if execute_process.returncode != 0:
        print("Hata: Program çalıştırılamadı.")
        print("Hata çıktısı:")
        print(execute_process.stderr)
        return
    
    # Program çıktısını yazdırma
    print("Program çıktısı:")
    print(execute_process.stdout)


# Test fonksiyonunu çağırma
run_tests()


