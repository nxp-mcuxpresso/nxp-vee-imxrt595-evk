/*
 * Java
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
package ej.fp.widget;

import ej.microui.event.EventCommand;

public class ButtonListener implements ej.fp.widget.Button.ButtonListener {

	@Override
	public void press(Button widget) {
		// do not send a Command on press event
	}

	@Override
	public void release(Button widget) {
		switch (widget.getID()) {
		case 0:
			EventCommand.sendEvent(EventCommand.UP);
			break;
		case 1:
			EventCommand.sendEvent(EventCommand.DOWN);
			break;
		}
	}
}
