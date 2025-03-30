Lorgnier Théo

__________________________

# Lesson 1 : UART Interactions

## MAKEFILE

Le makefile vient linker les fichiers .o fournie en créant le fichier kernel.elf

puis le fichier kernel.elf est transformer en binaire : kernel.bin

Pour executer le fichier .bin il faut renseigner en option des argument précisant:
le cpu (-cpu)
la mémoire (-m)
-nographic desactive le mode graphic et redirige les entrées sortie dans le terminal
-serial stdio permet de retenir les entrées et sorties depuis des composants réels
le mettre a null permetterait de renseigner des périphérique virtuel ou des interruptions "virtuel", on est alors en "Replay" alors que stdio est en record

-device loader charge le bin en mémoire, et device précise le composan que l'on souhaite utiliser

## GDB

Pour tester avec gdb dans ce cas, il faut lancer gdb sur un autre terminal en se connectant au port 1234 en localhost avec la commande (dans GDB): target remote :1234

gdb n'aime pas qu'on debug sur un autre systme, il faut alors lancer avec gdb-multiarch


## UART
dans la doc UART : 
(voir section 3.3.3 pour les masks de flags)
(voir section 3.2 pour les registres)

Le registre de status est appelé Flags register dans la doc obtenu, il peut avoir des variations de noms.

Dans ce registre il y a 8 bits interessant dont 2 qu'ils nous sont utiles : le bit 5 pour savoir si le TX est rempli et le bit 4 pour savoir si le RX est vide (respectivement au noms de TXFF,RXFE).

Les fonctions mmio_* permettent de lire ou écrire à l'adresse indiqué (voir main.h)
Des variantes sur une lecture de 8/16/32bits existe dans le cas où l'on souhaite plus d'un octets

__________________________

# Lesson 2 : Interruptions

Les interuptions nécessite plus d'action à réaliser pour les faire fonctionner.
Il faut pouvoir : la provoquer et la traiter

Il est important de savoir dicerner dans quelle documentation chercher pour éviter de perdre du temps ainsi que d'obtenir la bonne information. 
Si on doit chercher une information sur un détail de l'UART alors on regarde sur la documentation de l'UART cependant si on parle de son adresse de base, alors il faut ce réferer un une documentation d'un niveau plus haut comme celle de versatile_application_mainboard ou ARM926EJ

Malheuresement dans mon cas je n'ai pas réussi a faire fonctionner correctement les interuptions. Les détails seront expliqué plus tard.

## Provocation

### UART

Ces informations sont trouvable dans la documentation de l'UART

Pour provoquer l'interruption via l'UART0 il faut l'autoriser via UARTIMSC qui est un registre sur 32 bits. Dans notre cas on souhaite autoriser l'interruption lors de la reception, soit le bit 4 RXIM. Ce bit doit être mit à 1 pour autoriser l'interuption, mais si on souhaite plus tard la désactiver on peut mettre ce bit à 0. Une autre manière de retirer l'interuption est de mettre à 1 le bit au même offset dans le registre UARTICR.

Il faut aussi enregistrer le pointeur d'une fonction permettant d'être appelé lors de l'interuption, il faut alors la lié avec UART0 du coté code C.

### VIC

Il faut également autoriser l'interuption au niveau du VIC. Ce VIC Vectored Interrupt Controller fait office d'interface avec les interuptions de périphérique. Il centraliser les interuptions pour que l'on puisse déterminer la source de celle-ci et il également possible d'obtenir la priorité d'une interuptions si on souhaite autoriser les interuptions dans les interuptions, ce qui n'est pas dans notre cas.

Pour autoriser UART0 dans le VIC il faut mettre à 1 le bit 12 dans le registre VICINTENABLE. Il est necessaire de faire ça car la documentation indique que le VIC applique un ET logique avec le masque VICINTENABLE pour connaitre les interuptions autorisé.

Dans le VIC nous avons la possibilité de choisir 2 niveaux d'interuptions : FIQ et IRQ
Le FIQ permetterait une interuption plus rapide en évitant d'identifier la source de l'interruption et l'enregistrement de contexte.

Dans notre cas IRQ est utilisé pour son utilisation général comme expliqué dans la documentaton. On peut alors identifier la source de l'interuption et enregister le contexte.


VIC regorge de détails suplémentaire non demandé dans ce travail, il est important de ne pas utiliser ces notes pour utiliser le VIC mais pour comprendre son utilité et sa place lors d'une interuption.

## Traitement

### _isr_handler
Lorsque le VIC lève l'interuption, le handler _isr_handler en assembleur est appelé. Ce code assembleur permet de retenir le contexte actuel pour executer le code présent dans l'ISR. 

    sub lr, lr, #4
    stmfd sp!, {r0-r12, lr}
    bl isr
    ldmfd sp!, {r0-r12, pc}^

On fait un lr - 4 car d'après la documentation de cortex 8 il faut faire ceci lorsque la dernière instruction (ici lr en premmier paramètre) n'est pas executé car une interuption ce produit.

On enregistre avec stmfd, puis on execute la fonction ISR gérant les hanlers enregistrer en amont. Pour finir on restaure les registres avec ldmfd.

### isr

L'appel bl isr en assembleur appel une fonction défini en c dans notre fichier isr.c.

Dans cette cette fonction il est possible de déterminer qui est la source de l'interuption en regardant le register VICIRQSTATUS. Dans le cas où le bit est à 1 on execute la fonction enregistrer dans un tableau en c. 

Cependant il faut absolument refuser d'autre interuption pendant le traitement de celle-ci car nous ne gérons pas ici de priorité d'interuption ou d'interution multiple.

Pour ce faire on appel des fonctions en assembleur qui ajoute ou supprime le flag CPSR_IRQ_FLAG dans CPSR. L'ajouter permet de ne plus écouter les interuptions.

Il faut également mettre à 0 dans le VIC le bit avec l'offset associé à la source de l'interuption sinon il ne sera pas possible de re provoquer une interuption.

## Problèmes rencontrés

Lors des tests l'interuption ce produisait mais lorsque je debuggais avec GDB dans la fonction ISR il se passait 2 choses différentes : 

Premièrement lorsque la fonction core_disable_irqs finiser son excution, elle retourner directement dans reset_handler soit un PC à 0 ce qui est absurde.

Deuxièmement lorsque je ne faisait pas d'appel de sous fonction je bouclé a l'infini dans le for. En essayant de voir les valeurs des variables local, gdb indiquer une adresse abérante de la provenance de cette variable.

Dans ces 2 cas il y avait le même problème : la stack de l'interuption n'était pas allouer ou mal allouer.


En réalité après une nuit de sommeil j'ai réalisé de ne pas avoir appeler _irqs_setup qui initialise la irq_stack_top.


Une autre erreur était de clear le VIC après une interuption en utilisant ce registre : VICINTCLEAR. Du coup plus d'interuption se produit.

## Charactères spéciaux

Utiliser les flèches donne le résultat attendu, soit bouger le curseur à travers la console. Il se trouve qu'on peut absolument tous modifier dans la console, ce qui est normal puisqu'on demande bêtement a la console d'écrire sans contraintes.

En envoyant des octets avec un prefix de \0 il est possible d'écrire les caractères en utf-8. Donc dans ce code si on tape la lettre p, l'écran est clear car dans le cas de celle ci le code envoie "\033[H\033[J". (voir uart_receive)