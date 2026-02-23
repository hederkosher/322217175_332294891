[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$path = Join-Path (Get-Location) "AI_project.doc"
$word = New-Object -ComObject Word.Application
$word.Visible = $false
$doc = $word.Documents.Open($path)
$doc.Content.Text | Out-File -FilePath "AI_project_requirements.txt" -Encoding UTF8
$doc.Close($false)
$word.Quit()
[System.GC]::Collect()
Write-Host "Done. Check AI_project_requirements.txt"
