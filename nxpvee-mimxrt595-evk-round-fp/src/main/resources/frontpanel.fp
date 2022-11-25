<?xml version="1.0"?>
<!--
	Copyright 2019-2020 MicroEJ Corp. All rights reserved.
	Use of this source code is governed by a BSD-style license that can be found with this software.
-->
<frontpanel
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xmlns="https://developer.microej.com"
	xsi:schemaLocation="https://developer.microej.com .widget.xsd">

	<device name="NXP MIMXRT595" skin="Board.png">
		<ej.fp.widget.Display x="41" y="33" width="392" height="392" filter="mask_392.png" />
		<ej.fp.widget.Pointer x="41" y="33" width="392" height="392" filter="mask_392.png" touch="true"/>
		<ej.fp.widget.Button label="0" x="823" y="193" skin="button1.png" pushedSkin="button1_pushed.png" listenerClass="ej.fp.widget.ButtonListener"/>
		<ej.fp.widget.Button label="1" x="823" y="228" skin="button2.png" pushedSkin="button2_pushed.png" listenerClass="ej.fp.widget.ButtonListener"/>
	</device>
</frontpanel>

