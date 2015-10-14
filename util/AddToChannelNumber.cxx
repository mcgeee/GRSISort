#include "TChannel.h"
#include <map>

int main(int argc, char **argv) {
   if(argc != 3){
      printf("Usage: AddOneToChannel <int to add> <calfile.cal>\n");
      return 0;
   }

   int num_to_add = atoi(argv[1]);

   printf("Adding %d to all channels\n",num_to_add);

   //Read Cal file
   TChannel::ReadCalFile(argv[2]);
   std::map<unsigned int, TChannel*>::iterator it;
   std::map<unsigned int, TChannel*> * chanmap = TChannel::GetChannelMap();

   if(!chanmap){
      printf("can't find channel map\n");
      return 0;
   }
   
   std::vector<TChannel*> chanlist;

   for(it = chanmap->begin(); it != chanmap->end(); ++it){
      TChannel* chan = it->second;
      TChannel *newchan = new TChannel(chan);
      chanlist.push_back(newchan);
      newchan->SetNumber(newchan->GetNumber() + num_to_add);
   }

   TChannel::DeleteAllChannels();

   for(size_t i =0; i<chanlist.size();++i){
      TChannel::AddChannel(chanlist.at(i));
   }

   TChannel::WriteCalFile(argv[2]);

   return 1;
}
