# STM32exam
STM32 programming 
Si consideri un ascensore di cui si desidera gestire l’automazione. L’ascensore gesisce 9
piani. La scelta del piano a cui andare viene effettuata ruotando il trimmer su AN11, il quale
comporta la visualizzazione del piano selezionato [1, 9] sulla prima cifra del display, e confer mando con il tasto X. La selezione del piano provoca la partenza dell’ascensore verso il piano
selezionato, qualora l’ascensore non sia in moto, oppure la prenotazione del piano selezionato
qualora l’ascensore sia gia in corsa. `
La corsa dell’ascensore deve essere gestita nel seguente modo:


• Si chiudono le porte; tale evento dura 1.5 s, e indicato dal lampeggio del led rosso con `
periodo di 200 ms e sul display viene mostrata la dicitura CLOS.

• Si avvia la corsa dell’ascensore verso il piano; durante la corsa occorre visualizzare, sul
display, il piano via via raggiunto, simulando anche la presenza dell’ascensore tra due
piani; in particolare, quando l’ascensore si trova tra il piano P e il piano P + 1, occorre
visualizzare sul display il numero P seguito da un trattino;

• Durante la corsa occorre far lampeggiare il led giallo con periodo di 200 ms;

• La velocita dell’ascensore deve essere regolabile (vedi oltre); `

• All’arrivo al piano, si aprono le porte; tale evento dura 1.5 s, e indicato dal lampeggio `


del led verde con periodo di 200 ms e sul display viene mostrata la dicitura OPEn; a fine
apertura viene visualizzato di nuovo il numero del piano.
Alla pressione del tasto T il sistema deve entrare in uno stato di configurazione dal quale deve
essere possibile, tramite uno dei trimmer, regolare la velocita dell’ascensore in un intervallo `
compreso tra 1 sec/piano e 0.4 sec/piano, con granularita di ` 0.1 sec/piano (visualizzare su
display il valore via via selezionato). Ripremendo il tasto T si rientra nella normale operativita.`
Utilizzare la UART per stampare messaggi di log che segnalano gli eventi che accadono
durante l’operativita.`
