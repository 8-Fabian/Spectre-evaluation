# Spectre-evaluation
Evaluation of Spectre attack

## Prozessorkern isolieren
==/etc/default/grub== öffnen
==isolcpus=kerne== zu GRUB_CMDLINE_LINUX hinzufügen (https://docs.kernel.org/admin-guide/kernel-parameters.html)
```
sudo update-grub
reboot
```

## Evaluation ausführen
Python venv erstellen
```
cd build
python3 -m venv evaluation_venv
source evaluation_venv/bin/activate
pip install -r requirements.txt
python3 ./evaluation.py
```