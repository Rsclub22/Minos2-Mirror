#
# Digital Function Key File
#
# Edits may be necessary before using this file
#  In some contests - replace F1 expression CQ "TEST" with contest acronym
#    For example: WPX IOTA BARTG SPRINT    
#  Replace F6 and F9-11 Spare,  with contest exchange expressions
#    For example - Number? State? Zone?  
#    non-ESM - no other changes required 
#    ESM - RUN F2 - remove {ENTER} and first ! to avoid double-sending his call
#
# S&P F1 calls CQ and automatically places the program in RUN mode
# F2 F3 F4 F5 use "!" macro for his callsign
# F2 F7 {SENTRST} will default to 599 or allows manual entry of sig report
#
###################
#   RUN Messages
###################
F1 Run CQ,CQ CQ TEST {MYCALL} {MYCALL} {CALLFIELD}
F2 Run Exch,! {SENTRST} {EXCH} {EXCH} ! {SERIALFIELD}
F3 Run TU,{LOG}! TU de {MYCALL} QRZ? {CALLFIELD} 
F4 {MYCALL}, {MYCALL} 
F5 His Call, ! 
F6 Spare,
F7 My Exch,{SENTRST} {EXCH} {EXCH} 
F8 Agn?,agn? agn? 
F9 Log, {LOG},
F10 Call Field,{CALLFIELD}
F11 Serial Field,{SERIALFIELD}
F12 Wipe,{WIPE}
#
###################
#   S&P Messages
###################
# "&" doubled, displays one "&" in the button label
F1 S&&P CQ, CQ TEST {MYCALL} {MYCALL} CQ {CALLFIELD}
F2 S&&P Exch,! {SENTRST} {EXCH} {EXCH} {MYCALL} 
F3 S&&P TU,{ENTER}! TU de {MYCALL}{LOG} 
F4 S&&P Call Him,! de {MYCALL} {MYCALL} 
F5 His Call, ! 
F6 {MYCALL}, {MYCALL} 
F7 My Exch,{SENTRST} {EXCH} {EXCH} 
F8 Agn?,agn? agn? 
F9 Log, {LOG},
F10 Call Field,{CALLFIELD}
F11 Serial Field,{SERIALFIELD}
F12 Wipe,{WIPE}
