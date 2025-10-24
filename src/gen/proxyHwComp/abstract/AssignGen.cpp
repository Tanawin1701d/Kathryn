//
// Created by tanawin on 22/6/2024.
//

#include "AssignGen.h"

#include "gen/proxyHwComp/module/moduleGen.h"

namespace kathryn{


    AssignGenBase::~AssignGenBase(){
        for (UpdateEvent* ude: translatedUpdateEvent){
            delete ude;
        }
    }


    void AssignGenBase::routeDep(){

        assert(_asb != nullptr);
        _asb->sortUpEventByPriority();
        for (UpdateEvent* realUde: _asb->getUpdateMeta()){
            auto* newEvent = new UpdateEvent();
            /////////////////////////////////
            ////////// route update condition
            /////////////////////////////////
            if (realUde->srcUpdateCondition != nullptr){
                Operable* conRouted =
                    _mdGenMaster->routeSrcOprToThisModule(realUde->srcUpdateCondition);
                newEvent->srcUpdateCondition = conRouted;
            }
            /////////////////////////////
            ////////// route update state
            /////////////////////////////
            if (realUde->srcUpdateState != nullptr){
                Operable* stateRouted =
                    _mdGenMaster->routeSrcOprToThisModule(realUde->srcUpdateState);
                newEvent->srcUpdateState = stateRouted;
            }
            /////////////////////////////
            ////////// route update value
            /////////////////////////////
            assert(realUde->srcUpdateValue != nullptr);
            Operable* updateValueRouted =
            _mdGenMaster->routeSrcOprToThisModule(realUde->srcUpdateValue);
            newEvent->srcUpdateValue = updateValueRouted;
            /////////////////////////////////////////////////////////
            ////////// fill static value that have not to be rerouted
            /////////////////////////////////////////////////////////
            newEvent->desUpdateSlice = realUde->desUpdateSlice;
            newEvent->priority       = realUde->priority;
            newEvent->subPriority    = realUde->subPriority;
            newEvent->clkMode        = realUde->clkMode;
            ///////////////////////////////////
            /////////// push to the pool system
            ///////////////////////////////////
            translatedUpdateEvent.push_back(newEvent);
        }
    }

    std::string AssignGenBase::assignOpBase(bool isClockSen){
        std::string retStr;
        retStr += "always @(" +
        retStr += isClockSen ? "posedge clk" : "*";
        retStr += ") begin\n";

        for (UpdateEvent* upd: translatedUpdateEvent){
            ////// occ stands for occur yet!
            bool isStateConOcc = false;
            retStr += "     if ( ";
            if (upd->srcUpdateState != nullptr){
                isStateConOcc = true;
                retStr += getOprStrFromOpr(upd->srcUpdateState);
            }

            if (upd->srcUpdateCondition != nullptr){
                if (isStateConOcc){
                    retStr += " && ";
                }
                isStateConOcc = true;
                retStr += getOprStrFromOpr(upd->srcUpdateCondition);
            }

            if (!isStateConOcc){
                retStr += " 1 ";
            }

            retStr += ") begin\n";
            retStr += "         ";
            retStr += assignmentLine(upd->desUpdateSlice, upd->srcUpdateValue, isClockSen);
            retStr += "\n";
            retStr += "     end\n";

        }
        retStr += "end\n";
        return retStr;
    }

    std::string AssignGenBase::assignmentLine(Slice desSlice, Operable* srcUpdateValue, bool isDelayedAsm){
        assert(srcUpdateValue != nullptr);
        std::string asmOpr = isDelayedAsm ? " <= " : " = ";
        return getOpr(desSlice) + asmOpr + getOprStrFromOprAndShinkMsb(srcUpdateValue, desSlice.getSize()) + ";";
    }

    bool AssignGenBase::cmpAssignGenBase(AssignGenBase* asgb,
                                         OUT_SEARCH_POL searchPol){
        assert(asgb != nullptr);
        if ( translatedUpdateEvent.size() != asgb->translatedUpdateEvent.size()){
            return false;
        }

        for (int idx = 0; idx < translatedUpdateEvent.size(); idx++){
            UpdateEvent* a = translatedUpdateEvent[idx];
            UpdateEvent* b = asgb->translatedUpdateEvent[idx];
            ////// compare each element
            if (
                (!cmpEachOpr(a->srcUpdateCondition,b->srcUpdateCondition,
                getModuleGen(), asgb->getModuleGen(), searchPol)) ||
                (!cmpEachOpr(a->srcUpdateState, b->srcUpdateState,
                getModuleGen(), asgb->getModuleGen(), searchPol)) ||
                (!cmpEachOpr(a->srcUpdateValue, b->srcUpdateValue,
                getModuleGen(), asgb->getModuleGen(), searchPol))
            ){
                return false;
            }
        }
        return true;

    }



}
