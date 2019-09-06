#!/usr/bin/env python
import sys
from pcbnew import *
 
 
#pcb = pcbnew.GetBoard()
#for module in pcb.GetModules():   
    #print "* Module: %s"%module.GetReference()
    #module.GetValueObj().SetVisible(False)      # set Value as Hidden
    #module.GetReferenceObj().SetVisible(True)   # set Reference as Visible
     

class HideModuleValue(FootprintWizardPlugin):
    def __init__(self):
        FootprintWizardPlugin.__init__(self)
        self.name = "FPC"
        self.description = "FPC Footprint Wizard"
        self.ClearErrors()

    # This method checks the parameters provided to wizard and set errors
    def PrintModules(self):
	pcb = pcbnew.GetBoard()
	for module in pcb.GetModules():   
	    print "* Module: %s"%module.GetReference()        
 
# create our footprint wizard
hide_val = HideModuleValue()
 
# register it into pcbnew
hide_val.register()