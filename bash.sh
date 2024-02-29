#!/bin/bash

# C programını derleme
gcc -Wall hw2.c -o shell

if [ $? -ne 0 ]; then
    echo "Hata: Kod derlenemedi."
    exit 1
fi

# Test edilecek komutlar ve argümanları
test_commands=(
    #"ls > cmd1.txt",
    #"echo 'aaa,bb,cc,dd' | grep 'aa'",
    "q",
    #"grep 'portakal' searchFile.txt",
    #"cat file.txt",
    #"sed 's/apple/orange/g' searchFile.txt > cmd2.txt"
)

for command in "${test_commands[@]}"; do
    # C programını çalıştırma
    output=$(./shell <<<"$command" 2>&1)
    return_code=$?
    
    if [ $return_code -eq 0 ]; then
        echo "$command komutu başarıyla çalıştırıldı."
        echo "Çıktı:"
        echo "$output"
    else
        echo "$command komutu çalıştırılamadı."
        echo "Hata çıktısı:"
        echo "$output"
    fi
done


