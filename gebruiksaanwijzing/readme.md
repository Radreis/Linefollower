# Gebruiksaanwijzing

### vervangen batterijen
Wanneer de batterijen plat zijn moeten ze handmatig worden vervangen.

### draadloze communicatie
#### verbinding maken
Dit is voorzien maar jammer genoeg werkte de module niet bij mij. Indien de module werkt moet u eerst verbinding maken via bluetooth met "HC-06", indien u hem niet vindt kan het zijn dat u uw bluetooth instellingen moet aanpassen naar "geavanceerd". dan hebt u een bluetooth terminal nodig op het device waar u commando's van wilt versturen (laptop, smartphone...). Eenmaal u in de terminal verbonden bent bent u klaar om commando's te versturen!

#### commando's
debug [on/off]  
start  
stop  
set cycle [µs]  
set power [0..255]  
set diff [0..1]  
set kp [0..]   
calibrate black  
calibrate white  

### kalibratie
doordat het zwart van het circuit na verloop van tijd minder zwart is of het wit minder wit is dan in het begin kan men door elke keer voor men de linefollower laat runnen eerst te laten kalibreren verdere problemen vermijden. Bij het veranderen van parcour is het ook sterk aan te raden om eerst te kalibreren!

### settings
De robot rijdt stabiel met volgende parameters:  
power = 50
cycle time = 2500
kp = 20
diff = 0.40

### schakelaar
achteraan is er een schakelaar voorzien waarmee we de linefollowr handmatig kunnen stoppen, bij het terug inschakelen moet er een commando "run" verstuurd worden voordat de linefollower terug begint te rijden.
