================================================================================

TODO:
-Player(windows wdm player, ASIO player)
	Multiple audio card must be selectable same time.
-Mixer
	Egyedi vumeter view
-Egyszerû használhatóság,GRABBER.
 -disk file nevét írjuk be autómatikusan grabbelés után.
   Ehhez tudni kéne mikor fejezõdik be?

 -csak diskek legyenek a listában, ;s csak olyanok amiknek egy particiója van?
  hozni haza disket, kitalálni mi alapján. Pl beleolvashatnánk, validálhatnánk?
 -legyen két lista?
 -exit code  ::getExitCodeProcess()
 -interface-t deffiniálni shellexecute és createprocess féle elinditásokra.
 http://msdn.microsoft.com/en-us/library/ms682499.aspx
 http://support.microsoft.com/kb/q190351/ 	
 
 -Annotation lista custom (user leszúrhat kommenteket?)
 -Annotation lista autómenedzsmentes(Midi bar, perc-másodperc n darab ami látható)
 -Régiólista (elnevezhetõséggel)
 -Feladatlista (undo,redo-zható) kivágás, némitás hogyan?
  
 -File veget lejátszva auidio buffert elõbb szüntetjük meg mint ahogy a windows wdm
 befejezné a buffer feldolgozást. Meg kell várni mig leállt a hang...
 
 -open dialogban .img file -t is engedjen, és new documentként füzze be.
 -vágás téma..
 - IOCTL -es megoldást kinyomozni, saját dd-t lib formájában.
 - ogg, mp3 olvasas iras
 
 -peekfile is lehetne particionalt, igy tudnank a particios tablat szerkeszteni is.
 -legyen megjelenítve a particio valtas a wave-en.
 
 
 -Ha engedjük a particionálást, 
	-a map view nem jo helyen jelzi???
	-A cursort nem engedi valaminél tovább (maxlen?) pedig van ott még logical pos.
	-kitalálni valami tesztet a peekfile tartalmának tesztelésére, és particiók helyének tesztelésére.
	-csinálni egy generátort? vagy régióból exportálni egy másik kisebb fdms2 image-t?
	
  
  -Property dialogban mindig az utolso disk valasztodik ki.