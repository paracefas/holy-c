; ModuleID = 'paracefas_module'
source_filename = "paracefas_module"
target triple = "x86_64-pc-windows-msvc"

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 4, ptr %x, align 4
  %y = alloca i32, align 4
  store i32 7, ptr %y, align 4
  %x1 = load i32, ptr %x, align 4
  %y2 = load i32, ptr %y, align 4
  %addtmp = add i32 %x1, %y2
  %z = alloca i32, align 4
  store i32 %addtmp, ptr %z, align 4
  %z3 = load i32, ptr %z, align 4
  ret i32 %z3
}
