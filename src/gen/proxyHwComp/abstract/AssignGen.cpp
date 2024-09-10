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
        isDesAssignFullyEqual = _asb->checkDesIsFullyAssignAndEqual();

        for (UpdateEvent* realUde: _asb->getUpdateMeta()){
            auto* newEvent = new UpdateEvent();
            ////////// route update condition
            if (realUde->srcUpdateCondition != nullptr){
                Operable* conRouted =
                    _mdGenMaster->routeSrcOprToThisModule(realUde->srcUpdateCondition);
                newEvent->srcUpdateCondition = conRouted;
            }
            ////////// route update state
            if (realUde->srcUpdateState != nullptr){
                Operable* stateRouted =
                    _mdGenMaster->routeSrcOprToThisModule(realUde->srcUpdateState);
                newEvent->srcUpdateState = stateRouted;
            }
            ////////// route update value
            assert(realUde->srcUpdateValue != nullptr);
            Operable* updateValueRouted =
            _mdGenMaster->routeSrcOprToThisModule(realUde->srcUpdateValue);
            newEvent->srcUpdateValue = updateValueRouted;
            newEvent->desUpdateSlice = realUde->desUpdateSlice;
            newEvent->priority       = realUde->priority;
            translatedUpdateEvent.push_back(newEvent);
        }
    }

    std::string AssignGenBase::assignOpBase(bool isClockSen){
        std::string retStr;
        retStr += "always @(" +
        retStr += isClockSen ? "posedge clk" : "*";
        retStr += ") begin\n";

        if (isDesAssignFullyEqual){
            retStr += assignOpBase_chainCondition(isClockSen);
        }else{
            retStr += assignOpBase_soleCondition(isClockSen);
        }

        retStr += "end\n";
        return retStr;
    }

    std::string AssignGenBase::assignOpBase_chainCondition(bool isClockSen){

        std::string retStr;
        std::vector<std::vector<UpdateEvent*>> updateEvents =
        grpEventByPriorityValueAndPriority(translatedUpdateEvent);

        bool ifBlockDetect = false;

        for (auto r_iter_begin  = updateEvents.rbegin();
                  r_iter_begin != updateEvents.rend();
                  r_iter_begin++){

            std::vector<UpdateEvent*>& updateEventGrp = *r_iter_begin;
            std::string conStr;

            ////// manage the group condition
            int conIdx = 0;
            for (UpdateEvent* updateEvent : updateEventGrp){
                bool isSubConOccur = false;
                conStr += "(";
                //// update state
                if (updateEvent->srcUpdateState != nullptr){
                    conStr += getOprStrFromOpr(updateEvent->srcUpdateState);
                    isSubConOccur = true;
                }
                //// update condition
                if (updateEvent->srcUpdateCondition != nullptr){
                    if (isSubConOccur){
                        conStr += " && ";
                    }
                    conStr += getOprStrFromOpr(updateEvent->srcUpdateCondition);
                    isSubConOccur = true;
                }
                //// dummy result;
                if (!isSubConOccur){
                    conStr += " 1 ";
                }
                conStr += ")";
                conIdx++;
                if (conIdx < updateEventGrp.size()){
                    conStr += " || ";
                }
            }

            //// put the assign condition

            if (!ifBlockDetect){
                retStr += "     if ( ";
                ifBlockDetect = true;
            }else{
                retStr += "  else if (";
            }
            retStr += conStr + " ) begin";

            retStr += " //// ";
            retStr += " priority " + std::to_string(updateEventGrp[0]->priority);
            retStr += "\n";

            retStr += "         ";
            retStr += assignmentLine(updateEventGrp[0]->desUpdateSlice,
                                     updateEventGrp[0]->srcUpdateValue,
                                     isClockSen);
            retStr += "\n";
            retStr += "     end";

        }
        retStr += "\n";
        return retStr;

    }


    std::string AssignGenBase::assignOpBase_soleCondition(bool isClockSen){
        std::string retStr;
        for (UpdateEvent* upd: translatedUpdateEvent){
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
        return retStr;
    }


    std::string AssignGenBase::assignmentLine(Slice desSlice, Operable* srcUpdateValue, bool isDelayedAsm){
        assert(srcUpdateValue != nullptr);
        std::string asmOpr = isDelayedAsm ? " <= " : " = ";
        return getOpr(desSlice) + asmOpr + getOprStrFromOpr(srcUpdateValue) + ";";
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
