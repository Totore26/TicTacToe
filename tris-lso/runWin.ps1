# Chiedi all'utente il numero di client
Write-Host "Inserisci il numero di client da avviare"
$n_client = Read-Host

# Verifica se il numero di client è un numero positivo
if (-not ($n_client -as [int]) -or $n_client -lt 1) {
    Write-Host "❌ Riprova, inserisci un numero valido di client (numero intero positivo)"
    exit
}

$projectName = "tris-lso"
$clientName = "client"
$serverName = "server"

Write-Host "🧹 Controllo e rimozione di eventuali container esistenti..."

# Rimozione dei container client esistenti
for ($i = 1; $i -le 50; $i++) {
    $container = "$projectName-$clientName-$i"
    $exists = docker ps -a --format "{{.Names}}" | Where-Object { $_ -eq $container }
    if ($exists) {
        Write-Host "⚠️  Rimozione container client esistente: $container"
        docker rm -f $container | Out-Null
    }
}

# Rimozione del container server se esistente
$serverContainer = "$projectName-$serverName-1"
$serverExists = docker ps -a --format "{{.Names}}" | Where-Object { $_ -eq $serverContainer }
if ($serverExists) {
    Write-Host "⚠️  Rimozione container server esistente: $serverContainer"
    docker rm -f $serverContainer | Out-Null
}

# Avvia docker-compose
Write-Host "🚀 Avvio dei container..."
Start-Process powershell -ArgumentList "-NoExit", "-Command", "docker-compose up --build --scale client=$n_client"

# Aspetta che tutti i container siano avviati
$allContainersRunning = $false
while (-not $allContainersRunning)
{
    $containers = docker ps --format "{{.Names}}"
    $allContainersRunning = $true

    for ($i = 1; $i -le $n_client; $i++) 
    {
        $containerName = "$projectName-$clientName-$i"
        if (-not ($containers -contains $containerName)) 
        {
            $allContainersRunning = $false
            break
        }
    }

    if (-not $allContainersRunning)
    {
        Start-Sleep -Seconds 2
    }
}

# Avvia una finestra PowerShell per ogni client
for ($i = 1; $i -le $n_client; $i++)
{
    Start-Process powershell -ArgumentList "-NoExit", "-Command", "docker attach $projectName-$clientName-$i"
}

Write-Host "✅ Tutto pronto! I container sono stati avviati e connessi."
