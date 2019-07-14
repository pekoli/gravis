/************
 * gravis.h *
 ************
 * Copyright (C) 2003 Peter Kolb
 * kolb@ling.uni-potsdam.de 
 *
 * Dieses Programm ist freie Software. Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation
 * veröffentlicht, weitergeben und/oder modifizieren, entweder gemäß Version
 * 2 der Lizenz oder (nach Ihrer Option) jeder späteren Version.
 *
 * Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, daß es
 * Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, sogar ohne die
 * implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT FÜR EINEN
 * BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License.
 *
 * Sie sollten ein Exemplar der GNU General Public License zusammen mit
 * diesem Programm erhalten haben. Falls nicht, schreiben Sie an die Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA.
 */

/* Polyline Objekt: 2 
 * Subtyp Polyline: 1 (4: Box mit abgerundeten Ecken ARC-BOX)
 * line style:      0
 * Dicke:           1
 * Farbe:           0 (Schwarz)
 * Fuellfarbe:      7 
 * Tiefe:          50
 * Pen Style:       0 (not used)
 * area_fill:      -1 (no fill)
 * style_val:      0.000
 * join_style:      0
 * cap_style:       0
 * radius:         -1 (fuer ARC-BOX: 7) 
 * forward_arrow:   0
 * backward_arrow:  0
 * npoints:         2 (Linie: 2, ARC_BOX: 5, erster Punkt wird am Ende wiederholt)
 */
char polyline_h1[] = "2 1 0";
char polyline_h2[] = "0 7 50 0 -1 0.000 0 0 -1 0 0 2";
char  arc_box_h[] = "2 4 0 1 0 7 40 0 -1 0.000 0 0 7 0 0 5";

/* TEXT:            4
 * sub_type         0: linksbuendig, 1: mitte, 2: rechtsbuendig
 * Farbe:           0 (Schwarz)
 * Tiefe:          30 (je hoeher, desto weiter unten)
 * pen_style:       0 (not used)
 * font:            0 (default font)
 * font_size:      12 (Font size in points)
 * angle:           0.000
 * font_flags:      4 (Bitvektor)
 * Height:        135 (fuer font-Size=12)
 * Length:        ??? (approx. font_size*strlength)
 * x, y:          Koordinaten abhaengig von sub_type
 * char:          String\001
*/
char text_h[] = "4 1 0 30 0 0 12 0.0000 4 135";

/* ELLIPSE:         1
 * sub_type:        1: Radius, 2: Durchmesser
 * line_style:      0
 * Dicke:           1
 * pen_color:       0 (Schwarz)
 * fill_color:      7 (Weiss)
 * Tiefe:           40
 * pen_style:       0
 * area_fill:       -1:no fill; 20: weiss (falls fill_color=7)
 * style_val:       0.000
 * direction:       1
 * angle:           0.000
 * center x, center y
 * radius x, radius y
 * start x , start y (first point entered)
 * ende x, ende y    (last point entered)
*/
char ellipse_h[] = "1 2 0 1 0 7 40 0 20 0.000 1 0.0000";
