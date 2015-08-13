
#include "TPPG.h"

ClassImp(TPPGData)
ClassImp(TPPG)

TPPGData::TPPGData(){
   Clear();
}

TPPGData::TPPGData(const TPPGData& rhs) {
  ((TPPGData&)rhs).Copy(*this);
}

void TPPGData::Copy(TObject &rhs) const {
  ((TPPGData&)rhs)     = *this;
}

void TPPGData::SetTimeStamp() {
   Long64_t time = GetHighTimeStamp();
   time  = time << 28;
   time |= GetLowTimeStamp() & 0x0fffffff;
   ftimestamp = time;
}

void TPPGData::Clear(Option_t* opt) {
//Clears the TPPGData and leaves it a "junk" state. By junk, I just mean default
//so that we can tell that this PPG is no good.
   ftimestamp        =  0;
   fold_ppg          =  0xFFFF;
   fnew_ppg          =  0xFFFF;         
   fNetworkPacketId  =  -1;
   flowtimestamp     =  0;
   fhightimestamp    =  0;
}

void TPPGData::Print(Option_t* opt) const{
   printf("time: %7lld\t PPG Status: 0x%07x\t Old: 0x%07x\n",GetTimeStamp(),fnew_ppg,fold_ppg); 
}

TPPG::TPPG(){
   fPPGStatusMap = new PPGMap_t;
   this->Clear();
}

TPPG::TPPG(const TPPG& rhs){
   rhs.Copy(*this);
}

TPPG::~TPPG(){
   if(fPPGStatusMap){
      PPGMap_t::iterator ppgit;
      for(ppgit = fPPGStatusMap->begin(); ppgit != fPPGStatusMap->end(); ppgit++){
         if(ppgit->second){
            delete ppgit->second;
         }
         ppgit->second = 0;
      }
      delete fPPGStatusMap;
   }
   fPPGStatusMap = 0;
}

void TPPG::Copy(TObject &rhs) const {
   ((TPPG&)rhs)     = *this;
   ((TPPG&)rhs).fcurrIterator = ((TPPG&)rhs).fPPGStatusMap->begin();
}

Bool_t TPPG::MapIsEmpty() const {
//Checks to see if the ppg map is empty. We need this because we need to put a default
//PPG in at time T=0 to prevent bad things from happening. This function says the map
//is empty when only the default is there, which it essentially is.
   if(fPPGStatusMap->size() ==1)//We check for size 1 because we always start with a Junk event at time 0.
      return true;
   else 
      return false;
}

void TPPG::AddData(TPPGData* pat){
//Adds a PPG status word at a given time in the current run. Makes a copy of the pointer to
//store in the map.
   fPPGStatusMap->insert(std::make_pair(pat->GetTimeStamp(),new TPPGData(*pat)));
}

ULong64_t TPPG::GetLastStatusTime(ULong64_t time,ppg_pattern pat,bool exact_flag){
//Gets the last time that a status was given. If the ppg_pattern kJunk is passed, the 
//current status at the time "time" is looked for. If exact_flag is false, the bits of "pat" 
//are looked for and ignore the rest of the bits in the sotred statuses. If "exact_flag" 
//is true, the entire ppg pattern "pat" must be met.
   if(MapIsEmpty()){
      printf("Empty\n");
      return 0;
   }

   PPGMap_t::iterator curppg_it = --(fPPGStatusMap->upper_bound(time));
   PPGMap_t::iterator ppg_it;
   if(pat == kJunk){
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->begin(); --ppg_it){
         if(curppg_it->second->GetNewPPG() == ppg_it->second->GetNewPPG() && curppg_it != ppg_it ){
            return ppg_it->first;
         }
      }
   }
   else{
      for(ppg_it = curppg_it; ppg_it != fPPGStatusMap->begin(); --ppg_it){
         if(exact_flag){
            if(pat == ppg_it->second->GetNewPPG()){
               return ppg_it->first;
            }
         }
         else{
            if(pat | ppg_it->second->GetNewPPG()){
               return ppg_it->first;
            }
         }
      }
   }
   printf("No previous status\n");
   return 0;
}

uint16_t TPPG::GetStatus(ULong64_t time) const {
//Returns the current status of the PPG at the time "time".
   if(MapIsEmpty()){
      printf("Empty\n");
   }
   //The upper_bound and lower_bound functions always return an iterator to the NEXT map element. We back off by one because we want to know what the last PPG event was.
   return (uint16_t)((--(fPPGStatusMap->upper_bound(time)))->second->GetNewPPG());
}

void TPPG::Print(Option_t *opt) const{
   if(MapIsEmpty()){
      printf("Empty\n");
   }
   else{
      PPGMap_t::iterator ppgit;
      printf("*****************************\n");
      printf("           PPG STATUS        \n");
      printf("*****************************\n");
      for(ppgit = MapBegin(); ppgit != MapEnd(); ppgit++){
         ppgit->second->Print();
      }
   }
}

void TPPG::Clear(Option_t *opt){
   fPPGStatusMap->clear();
   //We always add a junk event to keep the code from crashing if we ask for a PPG below the lowest PPG time.
   AddData(new TPPGData);
   fcurrIterator = fPPGStatusMap->begin();
}

bool TPPG::Correct() {
   
   
   return true;
}

TPPGData* const TPPG::Next() {
   if(fcurrIterator != MapEnd()){
      return (++fcurrIterator)->second;
   }
   else{
      printf("Already at last PPG\n");
      return 0;
   }
}

TPPGData* const TPPG::Previous() {
   if(fcurrIterator != MapBegin()){
      return (--fcurrIterator)->second;
   }
   else{
      printf("Already at first PPG\n");
      return 0;
   }
}

TPPGData* const TPPG::Last(){
   fcurrIterator = MapEnd();
   --fcurrIterator;
   return fcurrIterator->second;
}

TPPGData* const TPPG::First(){
   fcurrIterator = MapBegin();
   return fcurrIterator->second;
}

void TPPG::Streamer(TBuffer &R__b)
{
   // Stream an object of class TPPG.

   if (R__b.IsReading()) {
      R__b.ReadClassBuffer(TPPG::Class(),this);
      fcurrIterator = fPPGStatusMap->begin();
   } else {
      R__b.WriteClassBuffer(TPPG::Class(),this);
   }
}

/*const char* TPPG::ConvertStatus(ppg_pattern pattern){
   switch(pattern){
      case kBeamOn:        { return "Beam On";}
      case kDecay:         { return "Decay";}
      case kBackground:    { return "Background";}
      case kTapeMove:      { return "Tape Move";}
      default:             { return "Junk";}
   };


}*/
