================================================================================

TODO:
-Player(windows wdm player, ASIO player)
	Multiple audio card must be selectable same time.
-Mixer
	Egyedi vumeter view
-Egyszer� haszn�lhat�s�g,GRABBER.
 -disk file nev�t �rjuk be aut�matikusan grabbel�s ut�n.
   Ehhez tudni k�ne mikor fejez�dik be?

 -csak diskek legyenek a list�ban, ;s csak olyanok amiknek egy partici�ja van?
  hozni haza disket, kital�lni mi alapj�n. Pl beleolvashatn�nk, valid�lhatn�nk?
 -legyen k�t lista?
 -exit code  ::getExitCodeProcess()
 -interface-t deffini�lni shellexecute �s createprocess f�le elindit�sokra.
 http://msdn.microsoft.com/en-us/library/ms682499.aspx
 http://support.microsoft.com/kb/q190351/ 	
 
 -Annotation lista custom (user lesz�rhat kommenteket?)
 -Annotation lista aut�menedzsmentes(Midi bar, perc-m�sodperc n darab ami l�that�)
 -R�gi�lista (elnevezhet�s�ggel)
 -Feladatlista (undo,redo-zhat�) kiv�g�s, n�mit�s hogyan?
  
 -File veget lej�tszva auidio buffert el�bb sz�ntetj�k meg mint ahogy a windows wdm
 befejezn� a buffer feldolgoz�st. Meg kell v�rni mig le�llt a hang...
 
 -open dialogban .img file -t is engedjen, �s new documentk�nt f�zze be.
 -v�g�s t�ma..
 - IOCTL -es megold�st kinyomozni, saj�t dd-t lib form�j�ban.
 - ogg, mp3 olvasas iras
 
 -peekfile is lehetne particionalt, igy tudnank a particios tablat szerkeszteni is.
 -legyen megjelen�tve a particio valtas a wave-en.
 
 
 -Ha engedj�k a particion�l�st, 
	-a map view nem jo helyen jelzi???
	-A cursort nem engedi valamin�l tov�bb (maxlen?) pedig van ott m�g logical pos.
	-kital�lni valami tesztet a peekfile tartalm�nak tesztel�s�re, �s partici�k hely�nek tesztel�s�re.
	-csin�lni egy gener�tort? vagy r�gi�b�l export�lni egy m�sik kisebb fdms2 image-t?
	
  
  -Property dialogban mindig az utolso disk valasztodik ki.