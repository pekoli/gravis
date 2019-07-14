# gravis
Graphen-Visualisierung durch simuliertes Abkühlen (simulated annealing)

## Visualisierung von Graphen mit GRAVIS

### 0. Inhaltsverzeichnis
  1. Einleitung
  2. Installation
  3. Benutzung
  4. Grenzen der Software
  5. Ausgabedateien im fig-Format in andere Formate umwandeln
  6. Hintergrund und Funktionsweise
  7. Expertenoptionen
  8. Literatur

### 1. Einleitung

Das Programm GRAVIS (GRAphen-VISualisierung) liest die Beschreibung eines
Graphen in Form einer Textdatei ein und versucht den Graphen möglichst optimal
zu zeichnen, d.h. mit möglichst wenigen Überschneidungen von Kanten und
Knoten, so dass etwaige Cluster innerhalb des Graphen sichtbar werden.
Dazu wird ein heuristisches Optimierungsverfahren eingesetzt, das sogenannte
simulierte Abkühlen (engl. simulated annealing). 

GRAVIS erzeugt eine Ausgabedatei im Grafikformat **fig**. Dieses Format kann 
mit frei verfügbaren Konvertierungsprogrammen in andere Formate umgewandelt 
werden, darunter PDF und GIF (siehe unten). Außerdem kann es mit dem 
Zeichenprogramm xfig bearbeitet werden.

### 2. Installation

Geben Sie im Verzeichnis, in dem sich das `Makefile` befindet, ein:
```
make
```
um das Programm `gravis` zu kompilieren. Wenn Sie `gravis` systemweit zur 
Verfügung stellen wollen, geben Sie als Benutzer root
```
make install
```
ein, um gravis im Verzeichnis `/usr/local/bin` zu installieren.

### 3. Benutzung

Der Aufruf lautet:
```
gravis eingabedatei
```
Die Eingabedatei enthält die Beschreibung eines beliebigen markierten
ungerichteten Graphen.

Ein markierter ungerichteter Beispielgraph mit den fünf Knoten a, b, c, d und
e:
```
    d ---300.2--- a ---1.0--- b
    |	          |           |
   2.3	         2.5       10000.0   
    |	          |           |
    e	          c ----------/
```
Die Kanten sind mit Fließkommazahlen (Werten) markiert. Z.B. hat die Kante
zwischen d und e den Wert 2.3. GRAVIS zeichnet Kanten mit großen Werten dicker
als Kanten mit kleinen Werten.

Derselbe Graph in Matrixnotation:
```
        a     b     c     d     e
   a         1.0   2.5  300.2
   b   1.0      10000.0
   c   2.5 10000.0
   d  300.2                    2.3
   e                     2.3
```
Derselbe Graph kann auch durch die Aufzählung seiner Kanten beschrieben
werden:
```
    a b 1.0
    a c 2.5
    a d 300.2
    b c 10000.0
    d e 2.3
```
Dies ist das Eingabeformat für GRAVIS. Auf jeder Zeile der Eingabedatei steht
eine Kante. Sie wird definiert durch die beiden Knotennamen (getrennt durch
ein Leerzeichen) und der Markierung der Kante, ebenfalls durch ein Leerzeichen
vom zweiten Knotennamen abgetrennt (die Knotennamen dürfen also keine
Leerzeichen enthalten!). Die Reihenfolge der Knotennamen spielt keine Rolle. 
Die Kantenmarkierung muss aus einer Fließkommazahl bestehen.

### 4. Grenzen der Software

Die Graphen dürfen nicht zu groß sein. Mehr als 25-30 Knoten können nicht mehr
vernünftig dargestellt werden. 

Aufgrund des verwendeten Algorithmus mit zufälligen Anfangsgraphen sehen die
erzeugten Graphen bei jedem Durchgang anders aus. Es lohnt sich also, mehrere
Durchläufe auszuführen und das beste Ergebnis auszuwählen.

Manchmal bleibt der Algorithmus in einem lokalen Minimum stecken. In diesem
Fall ändert sich an den Ausgabewerten am Ende der Zeile nichts, wie bei dieser
Beispielausgabe:
```
	 t=135.000000    e=inf   (besser:23, bergauf:0)
	 t=121.500000    e=inf   (besser:0, bergauf:0)
	 t=109.350000    e=inf   (besser:0, bergauf:0)
	 t=98.415000     e=inf   (besser:0, bergauf:0)
	 t=88.573500     e=inf   (besser:0, bergauf:0)
	 t=79.716150     e=inf   (besser:0, bergauf:0)
	 t=71.744535     e=inf   (besser:0, bergauf:0)
```
Dann sollte der Aufruf einfach wiederholt werden.

### 5. Ausgabedateien im fig-Format in andere Formate umwandeln

Mit Hilfe des Programms `fig2dev` können die von GRAVIS erzeugten fig-Dateien
in andere Grafikformate umgewandelt werden, unter anderem gif, pdf und latex.

Der Aufruf 
```
    fig2dev -L gif dateiname.fig dateiname.gif
```
beispielsweise erzeugt aus einer fig-Datei ein gif-Bild.

### 6. Hintergrund und Funktionsweise

Es wurde der "simulated annealing"-Algorithmus aus [1] implementiert. Den Kern
des Verfahrens bildet eine Kostenfunktion, die definiert, wann ein Graph 
"schön" ist und wann nicht. Sie setzt sich aus fünf Termen zusammen:
  1. Abstand zwischen den Knoten. Wenn eine Kante zwischen zwei Knoten besteht, sollen die Knoten sich anziehen, wenn es keine Kante gibt, sollen sie sich abstoßen.
  2. Abstand der Knoten zum Bildrand.
  3. Gesamtlänge aller Kanten.
  4. Anzahl sich kreuzender Kanten.
  5. Abstand zwischen Knoten und Kanten.

Die Gesamtkosten eines Graphen ergeben sich aus der Addition der Terme. Die
Gewichtung eines Terms kann durch einen Faktor beeinflusst werden. Als günstig
haben sich die folgenden Faktoren erwiesen:
```
      kosten = t1 + t2 + 1,5 * t3 + 2 * t4 + t5
```
Am wichtigsten ist die Minimierung der Anzahl sich kreuzender Kanten (t4),
gefolgt von der Minimierung der gesamten Kantenlänge im Graphen (t3). Beide
Terme zusammen sorgen mit Term t1 dafür, dass "Klumpen" zusammenhängender
Wörter entstehen. Term t2 verhindert, dass die Knoten über den Bildrand
wandern, t5 ist dafür zuständig, dass möglichst keine Knoten auf 
eine Kante gesetzt werden.

Zur Berechnung der einzelnen Terme aus einem gegebenen Graphen habe ich
grundlegende geometrische Algorithmen zur Bestimmung des Abstand zweier
Punkte, des Schnittpunkts zweier Geraden usw. aus [2] implementiert.
Die Laufzeit des "simulated annealing"-Algorithmus wird von [1] mit 
O(E*V^2) angegeben, wobei V die Zahl der Knoten, E die Zahl der Kanten
bezeichnet.

### 7. Expertenoptionen

Beim simulierten Abkühlen handelt es sich um ein heuristisches
Optimierungsverfahren. Es funktioniert wie folgt.

Zu Beginn werden die Knoten des Graphen an zufällige Positionen gesetzt. Dann
wird ein Knoten zufällig ausgewählt, und in eine zufällige Richtung
verschoben. Die Weite der Verschiebung hängt von der Temperatur ab: je höher
die Temperatur, desto größer die Verschiebung. Die Idee ist, dass zu Beginn
(bei hohen Temperaturen) große Veränderungen erfolgen sollen, während am Ende
der Optimierung (bei niedrigen Temperaturen) nur noch ein Feintuning
stattfindet. Nach jeder Knotenverschiebung wird mittels der im vorhergehenden
Abschnitt beschriebenen Kostenfunktion ermittelt, ob die Gesamtkosten des
Graphen sich verringert haben. Wenn ja, wird die Verschiebung beibehalten,
wenn nicht, wird sie zurückgenommen. Pro Temperatur werden vt
Knotenverschiebungen durchgeführt. Dann wird die Temperatur um den Faktor
gamma gesenkt, und es werden erneut vt Knotenverschiebungen durchgeführt,
usw., solange bis at Temperaturen durchlaufen wurden.

Was bedeuten die Ausgaben von GRAVIS?
```
    t=135.000000    e=9426.293147   (besser:20, bergauf:21)
    t=121.500000    e=6334.860209   (besser:30, bergauf:12)
    t=109.350000    e=4268.674251   (besser:21, bergauf:9)
```
t bezeichnet die momentane Temperatur, e die Kosten. Die Angabe "besser:20"
besagt, dass von den vt Knotenverschiebungen während der jeweiligen Temperatur
20 zu einer Kostenverringerung geführt haben.

Um nicht in lokalen Kostenminima steckenzubleiben, darf der Algorithmus mit
einer gewissen Wahrscheinlichkeit auch dann eine Knotenverschiebung
beibehalten, wenn sie zu einer Erhöhung der Kosten geführt hat. Die Anzahl
dieser Fälle pro Temperatur wird mit "bergauf" angegeben. Je größer der Faktor
ba (siehe unten), desto häufiger tritt dieser Fall ein.

Optionen fuer Experten:
```
   -st   Starttemperatur	default = 150.0
   -gm   Gamma			default = 0.90
   -vt	 Anzahl der Knotenverschiebungen pro Temperatur
				default = 40
   -at	 Anzahl zu durchlaufender Temperaturen
				default = 60
   -ba   Faktor zur Steuerung der "bergauf"-Fälle
			        default = 0.5
```
Aufruf: `gravis Eingabedatei [Optionen]`

Die Optionen haben großen Einfluss auf die Laufzeit, insbesondere die Option
-vt!

### 8. Literatur

[1] Ron Davidson und David Harel (1996): Drawing Graphs Nicely Using Simulated Annealing. ACM Transactions on Graphics 15(4), S. 301-331.

[2] Robert Sedgewick (1992): Algorithmen in C. Addison-Wesley.
