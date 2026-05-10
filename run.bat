.\build\holy-c.exe .\examples\hello_world.hc
clang .\output.ll -o main.exe
.\main.exe; Write-Host "El codigo de salida es: $LASTEXITCODE"