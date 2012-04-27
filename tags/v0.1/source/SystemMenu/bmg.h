/*
Copyright (c) 2012 - Dimok and giantpune

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/
#ifndef BMG_H
#define BMG_H

#include <gctypes.h>
#include "utils/char16.h"

// simple implimentation of "ipl::message::Message"

//! the way nintendo does it, each string has a hardcoded index.
//! but they change indexes with different versions of the system menu.  this implimtation expects string
//! indexes from the bmg in v514.  it will try to determine which bmg it is given by checking the crc,
//! and then convert the requested index into one for the version of bmg it thinks it has

//! the whole index resolving stuff is pretty specific to the "ipl_message.bmg" files used in the system menu

class Bmg
{
public:
	Bmg( const u8* stuff = NULL, u32 len = 0 );

	bool SetResource( const u8* stuff, u32 len );

	u16 GetMessageCount() const { return inf1Header ? IndexResolver ? 458 : inf1Header->numMessages : 0; }

	// get a pointer to a u16 string for the given index
	//! expects a message index from the bmg files in system menu 514
	const char16* GetMessage( u16 index ) const;

	// this seems like a good enough place to put a global translator
	static Bmg *Instance()
	{
		if( !instance )
		{
			instance = new Bmg;
		}
		return instance;
	}

protected:
	// header is 0x20 bytes
	struct BmgHeader
	{
		u64 magic;		// "MESGbmg1"
		u32 size;		// total file size
		u32 sectionCnt;	// number of sections

		u32 pad1;		// 0x10 bytes of padding
		u32 pad2;
		u32 pad3;
		u32 pad4;
	}__attribute__((packed));

	// 0x10 bytes
	struct Inf1Header
	{
		u32 magic;			// INF1 - 0x494e4631
		u32 size;			// section size
		u16 numMessages;	// number of messages
		u16 entrySize;		// i guess this is the size of each entry in the INF1 section?
		u32 pad;
	}__attribute__((packed));

	// 0x8 bytes
	struct Dat1Header
	{
		u32 magic;		// DAT1 - 0x44415431
		u32 size;		// section size
	}__attribute__((packed));


	BmgHeader *bmgHeader;
	Inf1Header *inf1Header;
	Dat1Header *dat1Header;
	u16 (*IndexResolver)( u16 index );

	void Reset();

	static Bmg *instance;
};

/*
 strings from version 514 - NOE

0 "Disc Channel"
1 "Wii Menu"
2 "Start"
3 ""
4 "Please insert a disc."
5 ""
6 "Unable to read the disc.

Check the Wii Operations Manual
for help troubleshooting."
7 "An error has occurred. Press
the Eject Button and remove
the disc, then turn the Wii
console off and refer to the
Wii Operations Manual for
help troubleshooting."
8 "The Wii System Memory is
damaged. Refer to the
Wii Operations Manual
for help troubleshooting."
9 "Please connect a Nunchuk
to the Wii Remote."
10 "Please connect a Classic Controller
to the Wii Remote."
11 "Wii Message Board data was
corrupted. The data has been
repaired, but some messages or
addresses may have been lost."
12 "The system files are corrupted.
Please refer to the Wii Operations Manual
for help troubleshooting."
13 "Please connect a Classic Controller to
the Wii Remote or connect a Nintendo
GameCube Controller to your Wii console."
14 "Wii Options"
15 "Wii Message Board"
16 "Wii Menu"
17 "Calendar"
18 "Create Message"
19 "This Channel can't be used."
20 "Update"
21 "Update"
22 "Channel Time Remaining: "
23 "Less than a minute"
24 ":"
25 ""
26 "Your licence for this
Channel has expired."
27 "The system is operating
in maintenance mode.
The Wii Message Board
cannot be used."
28 ""
29 "The period of use for this
Channel has ended."
30 "Press the A Button."
31 ":"
32 ""
33 "This cannot be used in
maintenance mode."
34 "Photos can be sent only to
other Wii consoles."
35 "Back"
36 "Post"
37 "Quit"
38 "End"
39 "Send"
40 "Trash"
41 "Register"
42 "Send Message"
43 "Change
Nickname"
44 "Wii Number"
45 "Nickname"
46 "OK"
47 "Erase"
48 "Erase this data?"
49 "Wii Number"
50 "Nickname"
51 "Send"
52 "This SD Card must be formatted.
This will erase all data saved to
the SD Card. Is this OK?"
53 "Formatting the SD Card...
Do not remove the SD Card or
power off your Wii console."
54 "The SD Card has been formatted."
55 "The SD Card could not be
formatted. This SD Card
can't be used."
56 "The device inserted in the
SD Card slot cannot be used."
57 "The Wii Message Board
is approaching its data
limits. Old data is being
moved to the SD Card."
58 "Data is being moved to the
SD Card...
Do not remove the SD Card or
power off your Wii console."
59 "Wii Message Board
data has been moved
to the SD Card."
60 "The SD Card is full. The data
could not be moved."
61 "Data could not be moved.
Please check the SD Card."
62 "The SD Card is locked. To save,
move, or erase data, please
unlock the SD Card."
63 "E-mail Address"
64 "Erase this?"
65 "Is this OK?"
66 "This console's Wii Number"
67 "Reply"
68 "This information has been
added to your address book."
69 "Are you sure you want to quit?"
70 "Others"
71 "Enter a Wii Number."
72 "Enter an address."
73 "Enter a nickname."
74 "The address has been registered.
To exchange messages, you must both
register one another and configure
your Internet settings."
75 "Confirm mail address"
76 "Confirm Wii Number"
77 "Wii"
78 "Choose an address"
79 "Back"
80 "Your address book is full, so you
can't register a new address."
81 "That Wii Friend has been erased."
82 "That Wii Number is already registered."
83 "That e-mail address is already registered."
84 "This Wii Number is incorrect."
85 "You can attach a Mii."
86 "This Wii Number can't be registered."
87 "Registration to be confirmed.

You must register one another to
be able to exchange messages."
88 "This will launch the Internet Channel.
OK?"
89 "You do not have the Internet Channel."
90 "Unable to jump using your current
Internet Channel."
91 "You do not have this Channel."
92 "Opt Out"
93 "Opt out of:"
94 "This message service"
95 "All message services"
96 "Cancel"
97 "Are you sure?"
98 "You have chosen to opt out of
this Channel's message service."
99 "You have chosen to opt out of
message services for all
Channels and Games."
100 "There is a problem with WiiConnect24.
Transmission could not be halted.
Please try again later."
101 "Opt Out"
102 "Opt out of:"
103 "This message service"
104 "All message services"
105 "Cancel"
106 "Are you sure ?"
107 "You have chosen to opt out of
this Channel's message service."
108 "You have chosen to opt out of
message services for all
Channels and Games."
109 ""
110 "Mon"
111 "Tue"
112 "Wed"
113 "Thu"
114 "Fri"
115 "Sat"
116 "Sun"
117 ""
118 "January"
119 "February"
120 "March"
121 "April"
122 "May"
123 "June"
124 "July"
125 "August"
126 "September"
127 "October"
128 "November"
129 "December"
130 ""
131 ""
132 "Today's Play History"
133 "Memo"
134 "Address Book"
135 "Total Play Time"
136 "Today's Accomplishments"
137 "Others"
138 ":"
139 "← Add a Mii"
140 "Write a memo"
141 "Sending the message
to xxxxxxxxxx."
142 "Write a message"
143 "Sending message..."
144 "Messages Sent: "
145 "And"
146 "more"
147 "Your Wii Message Board is full.
Old messages will now be erased."
148 "Erasing...
Do not turn the power off."
149 "Finished erasing."
150 "Old Wii Message Board data
can be moved to an SD Card.
Please insert an SD Card."
151 "An SD Card process failed."
152 "Wii Message Board data has
been moved to the SD Card.
The SD Card is now full."
153 "Data could not be erased.
The Wii System Memory is damaged.
Please see support.nintendo.com
for more information.
"
154 "Your Wii System Memory is full.
Use the Data Management Screen
to manage your save data."
155 "Data could not be erased.
The Wii System Memory is damaged.
Please see support.nintendo.com
for more information.
"
156 "Blocks Open: "
157 "Welcome to the SD Card Menu!

Here, you can launch Channels
saved on SD Cards by temporarily
utilising the Wii System Memory."
158 "About Save Data

Save data cannot be loaded
directly from SD Cards. To use
save data stored on SD Cards, first
move it to the Wii System Memory."
159 "Caution!

When you see this icon appear, do
not remove the SD Card or turn off
the power on the Wii console."
160 "SD Card Menu"
161 "SD Card Menu"
162 "SD Card Menu"
163 "Next"
164 "Close"
165 "Back"
166 "About the SD Card Menu"
167 "Move"
168 "Wii Menu"
169 "Nothing is inserted in the
SD Card slot."
170 "Loading from the SD Card..."
171 "The device inserted in the
SD Card slot cannot be used."
172 "Launch this Channel?"
173 "Launching..."
174 "Failed to launch."
175 "This Channel cannot be launched on
this Wii console."
176 "This Channel could not be launched.
Manage your Wii System Memory
to launch this Channel."
177 "Moving data to the SD Card..."
178 "Copy"
179 "Close the SD Card Menu and open
the Data Management Screen in
Wii Options?

This requires **** blocks in
the Wii System Memory."
180 "There are no Channels that can be
automatically managed.

Use the Data Management Screen in
Wii Options to free up **** blocks of
space."
181 "There is not enough free space on the
SD Card.

Please manage the SD Card to free up
additional space."
182 "Please select the criteria for Channels the
Wii console will move to an SD Card."
183 "Not Launched Recently"
184 "Right side of the Wii Menu"
185 "Lots of Blocks"
186 "A Few Blocks"
187 "Auto Manage"
188 "Use Data Management"
189 "Erase"
190 "The Channels shown below have been
selected. Move them to an SD Card?"
191 "The SD Card is locked. To save,
move, or erase data, please
unlock the SD Card."
192 "There is still not enough available space
in Wii System Memory.

Please manage data on the Data
Management Screen in Wii Options."
193 "There are so many Channels stored on
this SD Card that not all of them can be
displayed on the SD Card Menu."
194 "This SD Card must be formatted."
195 "An SD Card process failed."
196 "Next"
197 "Finished moving."
198 "This SD Card is write-protected.
No changes can be made to the
order of Channels."
199 "This Channel cannot be launched because
there is a newer version available."
200 "Continue"
201 "About the SD Card Menu

On the SD Card Menu, you can
temporarily utilise the Wii
System Memory to launch
a Channel stored on an SD Card."
202 "
To view this information again,
go to the SD Card Menu and
select the icon shown here."
203 "(Blue Channels will be replaced on the SD Card
with the ones stored on your Wii Console.)"
204 "Searching..."
205 "This data cannot be moved."
206 "The data cannot be moved because
there is no Memory Card in Slot A."
207 "The data cannot be moved because
there is no Memory Card in Slot B."
208 "The device inserted in
Slot A cannot be used."
209 "The device inserted in
Slot B cannot be used."
210 "The Memory Card inserted in Slot A
cannot be used with this menu."
211 "The Memory Card inserted in Slot B
cannot be used with this menu."
212 "Data cannot be moved between two
different types of Memory Cards."
213 "The Memory Card you're trying
to move the data to already
contains a copy of this data."
214 "The Memory Card you're trying to move
the data to has no space available or
it has too many files saved to it."
215 "The data may not have been moved."
216 "Finished copying."
217 "This data cannot be copied."
218 "The data cannot be copied because
there is no Memory Card in Slot A."
219 "The data cannot be copied because
there is no Memory Card in Slot B."
220 "The device inserted in
Slot A cannot be used."
221 "The device inserted in
Slot B cannot be used."
222 "The Memory Card inserted in Slot A
cannot be used with this menu."
223 "The Memory Card inserted in Slot B
cannot be used with this menu."
224 "Data cannot be copied between two
different types of Memory Cards."
225 "The Memory Card you're trying
to copy the data to already
contains a copy of this data."
226 "The Memory Card you're trying to copy
the data to has no space available
or it has too many files saved to it."
227 "The data may not have been copied."
228 "The data was erased."
229 "The data may not have been erased."
230 "Nothing is inserted in Slot A."
231 "Nothing is inserted in Slot B."
232 "The device inserted in
Slot A cannot be used."
233 "The device inserted in
Slot B cannot be used."
234 "The Memory Card inserted in Slot A
cannot be used with this menu."
235 "The Memory Card inserted in Slot B
cannot be used with this menu."
236 "The Memory Card in Slot A must
be formatted. Format it now?"
237 "The Memory Card in Slot B must
be formatted. Format it now?"
238 "If the Memory Card is formatted,
all saved data will be erased.
Is this OK?"
239 "If the Memory Card is formatted,
all saved data will be erased.
Is this OK?"
240 "The Memory Card was formatted."
241 "The Memory Card may not have
been formatted."
242 ""
243 "Move to the Memory Card in Slot A?"
244 "Copy to the Memory Card in Slot A?"
245 "Move to the Memory Card in Slot B?"
246 "Copy to the Memory Card in Slot B?"
247 "Erase?"
248 "The data has been erased."
249 "Copying..."
250 "Moving..."
251 "Erasing..."
252 "Back"
253 "Data Management"
254 "Save Data"
255 "Channels"
256 "Wii"
257 "Nintendo
GameCube"
258 "Copy"
259 "Move"
260 "Erase"
261 "Copy to the SD Card?"
262 "Copying to the SD Card...
Do not remove the SD Card or
power off your Wii console."
263 "The data has been copied."
264 "The SD Card is full.
Data can't be copied."
265 "Data can't be copied.
Check the SD Card."
266 "The SD Card is locked. To save or
move data, please unlock the
SD Card."
267 "Move to the SD Card?"
268 "Moving data to the SD Card...
Do not remove the SD Card or
power off your Wii console."
269 "The data has been moved."
270 "The data could not be moved
because the SD Card is full."
271 "Data could not be moved. Please
check the SD Card."
272 "Copy to the Wii System Memory?"
273 "Copying data to the Wii System
Memory. Do not remove the
SD Card or power off your
Wii console."
274 "There is not enough space in
the Wii System Memory."
275 "Move to the Wii System Memory?"
276 "Moving data to the Wii System
Memory. Do not remove the
SD Card or power off your
Wii console."
277 "The Wii System Memory is full.
Data can't be moved."
278 "There is not enough free space
on the SD Card."
279 "The data is corrupted and cannot
be read."
280 "GB"
281 "MB"
282 "KB"
283 "Some data cannot be copied to the
SD Card. Would you like to continue?"
284 "This data cannot be copied."
285 "Since some data cannot be copied, erasing
it means that data cannot be recovered.
Would you like to erase it anyway?"
286 "This Channel cannot be copied or erased."
287 "This data already exists on the SD Card.
Replace the existing data with this file?"
288 "This data already exists in the
Wii System Memory. Replace the
existing data with this file?"
289 "Nothing is inserted in the SD Card slot."
290 "An SD Card process failed."
291 "Some data could not be copied.
For details, please refer to
the game's Instruction Booklet."
292 "You must first play this game on
your Wii Console to copy save data."
293 "An updated version of this Channel
is already saved to the Wii System Memory.
This Channel cannot be copied."
294 "There are not enough free blocks,
free files, or free Channels in
your Wii System Memory."
295 "This data already exists on the SD Card."
296 "This data already exists in the
Wii System Memory."
297 "There is already a file with this
name, so this file cannot be created."
298 "The data has been copied. This includes
some data that cannot be used on other
Wii consoles. Please refer to the
game's Instruction Booklet for details."
299 "Data has been copied. There was
some data that could not be copied
to an SD Card. Refer to the game's
Instruction Booklet for details."
300 "This Channel cannot be copied
to this Wii console."
301 "The data has been copied. This data cannot
be copied to another Wii console. See
the operations guide for more information."
302 "This data cannot be copied to this
Wii console."
303 "SD Card Menu Data"
304 "This Channel cannot be moved to this
Wii console."
305 "This file cannot be moved, but it can
be copied."
306 "The data has been moved. This includes
some data that cannot be used on other
Wii consoles. See the game's Instruction
Booklet for details."
307 "Some of the data could not be moved.
See the game's Instruction Booklet for
details."
308 "The data has been moved. However,
it cannot be moved to another Wii
console. See the game's Instruction
Booklet for details."
309 "This data cannot be moved to this
Wii console."
310 "You must first play this game on your
Wii Console to move save data."
311 "This data can't be moved because a
portion of it can't be copied."
312 "This Channel cannot be copied because
there is a newer version available."
313 "This Channel cannot be moved because
there is a newer version available."
314 "Erasing...
Do not remove the SD Card or
power off your Wii console."
315 "Back"
316 "Wii Settings"
317 "Wii"
318 "SD Card"
319 "Slot A"
320 "Slot B"
321 "Yes"
322 "No"
323 "An Internet connection is required.

Configure Internet settings now?"
324 "You must configure Internet settings
to send messages over the Internet."
325 "You must configure Internet settings
to register a Wii Friend."
326 "Enter Settings"
327 "Restricted by Parental Controls."
328 "Please input your PIN."
329 "Restrictions have been temporarily
removed. Restrictions will
be reactivated when you
return to the Wii Menu."
330 "Restrictions have been temporarily
removed. They will be reactivated
when you have finished."
331 "This PIN is incorrect."
332 "Restricted by Parental Controls.
Please confirm in Wii Settings."
333 "Enter your IP Address."
334 "Enter your Primary DNS."
335 "Enter your Secondary DNS."
336 "Enter the Subnet Mask."
337 "Enter your Gateway."
338 "Enter your SSID."
339 "Enter the key."
340 "Enter the name of your proxy server."
341 "Enter the port number."
342 "Enter your user name."
343 "Enter your password."
344 "Enter your MTU value."
345 "Enter your PIN."
346 "Enter the answer to your secret question."
347 "Enter the master key."
348 "Enter a nickname for your Wii."
349 "Enter the mail address."
350 "Enter a nickname."
351 "Remember, if you forget your PIN
or the answer to your secret question,
you will be unable to remove Parental
Controls restrictions."
352 "If you forget your PIN and the answer
to your secret question, you won't be
able to remove Parental Controls
restrictions."
353 "The connection test was
successful. Your Wii will
now be automatically updated."
354 "Updating... Do not turn the
power off. Please wait a moment."
355 "Wii update complete."
356 "The Wii update failed."
357 "In order to use this software, you
must perform a Wii system update.
Select OK to begin the update."
358 "Nintendo's Internet-based services
have not yet launched. Please check
back at a later date."
359 "There are no updates available for
this Wii since it already has the
most current system software."
360 "Performing a Wii system update. Please wait
a moment. In some cases it may take up to
xx minutes. Do not turn the power off."
361 "Next, a Wii system update and
Channel update will be initiated."
362 "The Wii system update is complete.
Parental Controls are available.
Please go to Wii Options from the
Wii Menu if you would like
to customize Parental Controls."
363 "Connect to the Internet and
perform a Wii system update?"
364 "To perform a Wii system
update, please configure
Internet settings under
Wii Settings."
365 "The connection test was successful.
Perform a Wii system update now?"
366 "Would you like to use the Wii Shop Channel
and WiiConnect24?"
367 "Are you sure?"
368 "To use WiiConnect24 and the
Wii Shop Channel, please confirm the
user agreement after configuring
Internet settings."
369 "Try Again"
370 "Please review the User Agreements
in Wii Settings to use the Wii Shop
Channel or WiiConnect24."
371 "Wii Parental Controls are available.
Please go to Wii Options from the
Wii Menu if you would like
to customize Parental Controls.
"
372 "To use WiiConnect24 and the
Wii Shop Channel, please confirm the
Wii Network Services Agreement after
configuring Internet settings."
373 "Please review the Wii Network Services
Agreement in Wii Settings to use the Wii
Shop Channel or WiiConnect24."
374 "IMPORTANT:  If your Wii console has any unauthorised
modification, this update may detect and remove
unauthorised content causing the immediate or delayed
inoperability of your console. If you do not wish to accept
this update, you can cancel it by holding"
375 "IMPORTANT:  If your Wii console has any unauthorised
modification, this update may detect and remove
unauthorised content causing the immediate or delayed
inoperability of your console. If you do not wish to accept
this update, you can cancel it by holding"
376 "I Accept"
377 "I Accept"
378 "IMPORTANT:  If your Wii console has any unauthorised
modification, this update may detect and remove
unauthorised content causing the immediate or delayed
inoperability of your console. Failure to accept this update
may also render this and future games un"
379 "In order to use this software, you
must perform a Wii system update.
Select OK to begin the update.

Please note that this update will add
new Channels to the Wii Menu."
380 "No Miis have been registered.
Please use the Mii Channel to
create a Mii."
381 "Unable to load Mii Channel data.
Please access the Mii Channel
and check Mii Channel data.
"
382 "WiiConnect24 is not turned on.
Confirm your WiiConnect24 setting
in Wii Settings."
383 "You must configure Internet
settings to perform a
Wii system update."
384 "You must perform a Wii system
update to use this functionality."
385 "Wii Friend Registration Mail"
386 "An owner of a Wii Console is asking permission
to register you as a Wii Friend to exchange
e-mail with you. This individual's Wii e-mail address is
w1234567890123456@wii.com <mailto:w1234567890123456@wii.com>

If you would like to exchange e-mail with this"
387 "Wii Message"
388 ""
389 ""
390 ""
391 ""
392 ""
393 ""
394 ""
395 ""
396 ""
397 ""
398 ""
399 ""
400 "Error Code: "
401 "Unable to connect to the server.
Please try again later. Visit
support.nintendo.com for assistance."
402 "Unable to perform the Wii system
update because of an Internet error.
Please try again later. Visit
support.nintendo.com for assistance."
403 "There is not enough empty space
in the Wii System Memory. Unable
to perform a Wii system update. Visit
support.nintendo.com for assistance."
404 "An error has occurred. Unable to
perform the Wii system update. Visit
support.nintendo.com for assistance."
405 "Unable to connect to the Internet. Visit
support.nintendo.com for assistance."
406 "No network connections have been
configured. Choose Internet settings,
then set up a network connection. Visit
support.nintendo.com for assistance."
407 "Internet settings are incorrect. Visit
support.nintendo.com for assistance."
408 "The LAN Adapter has not
been properly inserted. Visit
support.nintendo.com for assistance."
409 "Unable to find an access point. Please
confirm the SSID and try again. Visit
support.nintendo.com for assistance."
410 "Unable to connect to the Internet.
Choose Internet settings and then
confirm your connection. Visit
support.nintendo.com for assistance."
411 "Unable to connect to the Internet. Please
see the Wii Operations Manual or visit
support.nintendo.com for assistance."
412 "Unable to connect to the Internet. Please
see the Wii Operations Manual or visit
support.nintendo.com for assistance."
413 "Unable to connect to the Internet. Please
see the Wii Operations Manual or visit
support.nintendo.com for assistance."
414 "Unable to connect to the server.
Please try again later. Visit
support.nintendo.com for assistance."
415 "Unable to connect to the proxy server.
Visit support.nintendo.com for assistance."
416 "Unable to connect. The user name and
password are incorrect. Confirm
connection settings and input the
correct user name and password. Visit
support.nintendo.com for assistance."
417 "Unable to connect to the server.
Please try again later. Visit
support.nintendo.com for assistance."
418 "Unable to connect to the Internet because
another device has been set up using the
same IP Address. Confirm network
connection settings and try again. Visit
support.nintendo.com for assistance."
419 "A communication error has occurred. You've
been disconnected from the Internet. Visit
support.nintendo.com for assistance."
420 "Unable to find the Nintendo Wi-Fi USB
Connector. Please check the Nintendo Wi-Fi
USB Connector to ensure it is functioning.
Please visit support.nintendo.com for
more information."
421 "Unable to connect to the server.
Please try again later. Visit
support.nintendo.com for assistance."
422 "Unable to perform the Wii system
update because of an Internet error.
Please try again later. Visit
support.nintendo.com for assistance."
423 "There is not enough empty space
in the Wii System Memory. Unable
to perform a Wii system update. Visit
support.nintendo.com for assistance."
424 "An error has occurred. Unable to
perform the Wii system update. Visit
support.nintendo.com for assistance."
425 "Unable to connect to the Internet. Visit
support.nintendo.com for assistance."
426 "No network connections have been
configured. Choose Internet settings,
then set up a network connection. Visit
support.nintendo.com for assistance."
427 "Internet settings are incorrect. Visit
support.nintendo.com for assistance."
428 "The LAN Adapter has not
been properly inserted. Visit
support.nintendo.com for assistance."
429 "Unable to find an access point. Please
confirm the SSID and try again. Visit
support.nintendo.com for assistance."
430 "Unable to connect to the Internet.
Choose Internet settings and then
confirm your connection. Visit
support.nintendo.com for assistance."
431 "Unable to connect to the Internet. Please
see the Wii Operations Manual or visit
support.nintendo.com for assistance."
432 "Unable to connect to the Internet. Please
see the Wii Operations Manual or visit
support.nintendo.com for assistance."
433 "Unable to connect to the Internet. Please
see the Wii Operations Manual or visit
support.nintendo.com for assistance."
434 "Unable to connect to the server.
Please try again later. Visit
support.nintendo.com for assistance."
435 "Unable to connect to the proxy server.
Visit support.nintendo.com for assistance."
436 "Unable to connect. The user name and
password are incorrect. Confirm
connection settings and input the
correct user name and password. Visit
support.nintendo.com for assistance."
437 "Unable to connect to the server.
Please try again later. Visit
support.nintendo.com for assistance."
438 "Unable to connect to the Internet because
another device has been set up using the
same IP Address. Confirm network
connection settings and try again. Visit
support.nintendo.com for assistance."
439 "A communication error has occurred. You've
been disconnected from the Internet. Visit
support.nintendo.com for assistance."
440 "Unable to find the Nintendo Wi-Fi USB
Connector. Please check the Nintendo Wi-Fi
USB Connector to ensure it is functioning.
Please visit support.nintendo.com for
more information."
441 "Wii Support Code: "
442 "Please input a four-digit PIN."
443 "The answer to your secret question
must be at least six characters."
444 "Please input the five-digit master key."
445 "Be sure to set your TV to display in
widescreen format when choosing the
Widescreen (16:9) setting on your Wii."
446 "The information you entered
is incorrect. Please check the
information and try again."
447 "Aim the Wii Remote at the TV. Press
  or  to adjust sensitivity
until only two blinking lights appear."
448 "Enter a nickname for your Wii console."
449 "Please enter something besides blank spaces."
450 "AOSS setup failed. Move closer
or make sure the access point
is powered on. Then try again.
For help, visit
support.nintendo.com"
451 ""
452 "Parental Controls and WiiConnect24
settings must be reconfigured."
453 "There is a problem with
the WiiConnect24 service.
Please try again later."
454 "There is a problem with
WiiConnect24.
Please try again later."
455 "The action you wish to perform
cannot be completed at this time.
Please try again later."
456 "This Wii Friend has been erased
from your address book, so you
cannot reply to this message."
457 "Be sure to set your TV to display in
standard format when choosing the
Standard (4:3) setting on your Wii."
*/


#endif // BMG_H
