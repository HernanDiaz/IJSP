# Copia los pools crudos de D:\seeds a pools_test\ para las 61 instancias x 4 generadores.
$dst = "C:\Users\diazhernan\CLionProjects\RL Seeds\IJSP\pools_test"
$classes = @{ "tai15_15"=1..10; "tai20_20"=1..10; "tai30_15"=1..10; "tai30_20"=1..10; "tai50_15"=1..10; "tai50_20"=1..10 }
$gens = @("v2","graspmor","gtmwkr","gp")
$ok=0; $miss=0
foreach($cls in $classes.Keys){ foreach($i in $classes[$cls]){
  $inst = "{0}_{1:d2}" -f $cls, $i
  foreach($g in $gens){
    $f = "D:\seeds\int__${inst}_${g}_pool.csv"
    if(Test-Path $f){ Copy-Item $f $dst -Force; $ok++ } else { Write-Output "FALTA: $f"; $miss++ }
  }
}}
# ft10: probar nombres
foreach($g in $gens){
  $cands = @("D:\seeds\int__ft10_${g}_pool.csv", "D:\seeds\int__ft10_interval_${g}_pool.csv", "D:\seeds\int__ft10_10_${g}_pool.csv")
  $found = $false
  foreach($c in $cands){ if(Test-Path $c){ Copy-Item $c $dst -Force; $ok++; $found=$true; Write-Output "ft10 ${g}: $(Split-Path $c -Leaf)"; break } }
  if(-not $found){ Write-Output "FALTA ft10 ${g} (probados 3 nombres)"; $miss++ }
}
Write-Output "copiados=$ok  faltan=$miss"
