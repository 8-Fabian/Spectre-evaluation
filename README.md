# Spectre-evaluation
Evaluation of Spectre attack

## Prozessorkern isolieren
<mark>/etc/default/grub</mark> öffnen\
<mark>isolcpus=kerne</mark> zu GRUB_CMDLINE_LINUX hinzufügen (https://docs.kernel.org/admin-guide/kernel-parameters.html)
```
sudo update-grub
reboot
```

## Voraussetzung
cmake, nasm und stress-ng\
Python venv:
```
cd build
python3 -m venv evaluation_venv
source evaluation_venv/bin/activate
pip install -r requirements.txt
```

## Evaluation ausführen
```
python3 ./evaluation.py
```
