Lorgnier Théo

MAKEFILE:

Le makefile vient linker les fichiers .o fournie en créant le fichier kernel.elf

puis le fichier kernel.elf est transformer en binaire : kernel.bin

Pour executer le fichier .bin il faut renseigner en option des argument précisant:
le cpu (-cpu)
la mémoire (-m)
-nographic desactive le mode graphic et redirige les entrées sortie dans le terminal
-serial stdio permet de retenir les entrées et sorties depuis des composants réels
le mettre a null permetterait de renseigner des périphérique virtuel ou des interruptions "virtuel", on est alors en "Replay" alors que stdio est en record

-device loader charge le bin en mémoire, et device précise le composan que l'on souhaite utiliser

GDB:

Pour tester avec gdb dans ce cas, il faut lancer gdb sur un autre terminal en se connectant au port 1234 en localhost


UART:

Le registre de status est appelé Flags register dans la doc obtenu, il peut avoir des variations de noms.

Dans ce registre il y a 8 bits interessant dont 2 qui nous est utiles : le bit 5 pour savoir si le TX est rempli et le bit 4 pour savoir si le RX est vide.

Les fonctions mmio_* permettent de lire ou écrire a l'adresse indiqué (voir main.h)
Des variantes sur une lecture de 8/16/32bits existe dans le cas où l'on souhaite plus d'un octets