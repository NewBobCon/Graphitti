#include "AllSpikingNeurons.h"
#include "AllSpikingSynapses.h"
#include <vector>

// Default constructor
AllSpikingNeurons::AllSpikingNeurons() : AllNeurons() {
   hasFired_ = NULL;
   spikeCount_ = NULL;
   spikeCountOffset_ = NULL;
   spikeHistory_ = NULL;
}

AllSpikingNeurons::~AllSpikingNeurons() {
   if (size_ != 0) {
      for (int i = 0; i < size_; i++) {
         delete[] spikeHistory_[i];
      }

      delete[] hasFired_;
      delete[] spikeCount_;
      delete[] spikeCountOffset_;
      delete[] spikeHistory_;
   }

   hasFired_ = NULL;
   spikeCount_ = NULL;
   spikeCountOffset_ = NULL;
   spikeHistory_ = NULL;
}

/*
 *  Setup the internal structure of the class (allocate memories).
 *
 */
void AllSpikingNeurons::setupNeurons() {
   AllNeurons::setupNeurons();

   // TODO: Rename variables for easier identification
   hasFired_ = new bool[size_];
   spikeCount_ = new int[size_];
   spikeCountOffset_ = new int[size_];
   spikeHistory_ = new uint64_t *[size_];

   for (int i = 0; i < size_; ++i) {
      spikeHistory_[i] = NULL;
      hasFired_[i] = false;
      spikeCount_[i] = 0;
      spikeCountOffset_[i] = 0;
   }

   Simulator::getInstance().setPSummationMap(summationMap_);
}

/*
 *  Clear the spike counts out of all Neurons.
 */
void AllSpikingNeurons::clearSpikeCounts() {
   int maxSpikes = (int) ((Simulator::getInstance().getEpochDuration() * Simulator::getInstance().getMaxFiringRate()));

   for (int i = 0; i < Simulator::getInstance().getTotalNeurons(); i++) {
      spikeCountOffset_[i] = (spikeCount_[i] + spikeCountOffset_[i]) % maxSpikes;
      spikeCount_[i] = 0;
   }
}

#if !defined(USE_GPU)

/*
 *  Update internal state of the indexed Neuron (called by every simulation step).
 *  Notify outgoing synapses if neuron has fired.
 *
 *  @param  synapses         The Synapse list to search from.
 *  @param  synapseIndexMap  Reference to the SynapseIndexMap.
 */
void AllSpikingNeurons::advanceNeurons(IAllSynapses &synapses, const SynapseIndexMap *synapseIndexMap) {
   int maxSpikes = (int) ((Simulator::getInstance().getEpochDuration() * Simulator::getInstance().getMaxFiringRate()));
   //const BGSIZE *outgoingMapBegin=(synapseIndexMap->outgoingSynapseBegin_);
   //uint32_t *outgoingMapBegin1=(synapseIndexMap->outgoingSynapseBegin_).data();

   AllSpikingSynapses &spSynapses = dynamic_cast<AllSpikingSynapses &>(synapses);
   // For each neuron in the network
   for (int idx = Simulator::getInstance().getTotalNeurons() - 1; idx >= 0; --idx) {
      // advance neurons
      advanceNeuron(idx);

      // notify outgoing/incoming synapses if neuron has fired
      if (hasFired_[idx]) {
         LOG4CPLUS_DEBUG(neuronLogger_, "Neuron: " << idx << " has fired at time: "
                        << g_simulationStep * Simulator::getInstance().getDeltaT());

         assert(spikeCount_[idx] < maxSpikes);

         // notify outgoing synapses
         BGSIZE synapseCounts;


         if (synapseIndexMap != NULL) {
            synapseCounts = synapseIndexMap->outgoingSynapseCount_[idx];
            if (synapseCounts != 0) {
               int beginIndex = synapseIndexMap->outgoingSynapseBegin_[idx];
               BGSIZE iSyn;
               const BGSIZE *outgoingMapBegin=&(synapseIndexMap->outgoingSynapseIndexMap_[beginIndex]);
           
               for (BGSIZE i = 0; i < synapseCounts; i++) {
                  iSyn=outgoingMapBegin[i];
                  spSynapses.preSpikeHit(iSyn);
               }
            }
         }

         // notify incoming synapses
         synapseCounts = spSynapses.synapseCounts_[idx];
         BGSIZE synapse_notified = 0;

         if (spSynapses.allowBackPropagation()) {
            for (int z = 0; synapse_notified < synapseCounts; z++) {
               BGSIZE iSyn = Simulator::getInstance().getMaxSynapsesPerNeuron() * idx + z;
               if (spSynapses.inUse_[iSyn] == true) {
                  spSynapses.postSpikeHit(iSyn);
                  synapse_notified++;
               }
            }
         }

         hasFired_[idx] = false;
      }
   }
}

/*
 *  Fire the selected Neuron and calculate the result.
 *
 *  @param  index       Index of the Neuron to update.
 */
void AllSpikingNeurons::fire(const int index) const {
   // Note that the neuron has fired!
   hasFired_[index] = true;

   // record spike time
   int maxSpikes = (int) ((Simulator::getInstance().getEpochDuration() * Simulator::getInstance().getMaxFiringRate()));
   int idxSp = (spikeCount_[index] + spikeCountOffset_[index]) % maxSpikes;
   spikeHistory_[index][idxSp] = g_simulationStep;

   // increment spike count and total spike count
   spikeCount_[index]++;
}

/*
 *  Get the spike history of neuron[index] at the location offIndex.
 *  More specifically, retrieves the global simulation time step for the spike
 *  in question from the spike history record.
 *  
 *  TODO: need to document clearly how spikeHistory_ is updated, when/if it gets
 *  cleared/dumped to file, and how the actual index (idxSp) computation works,
 *  as it is very unobvious.
 *
 *  @param  index            Index of the neuron to get spike history.
 *  @param  offIndex         Offset of the history buffer to get from.
 */
uint64_t AllSpikingNeurons::getSpikeHistory(int index, int offIndex) {
   // offIndex is a minus offset
   int maxSpikes = (int) ((Simulator::getInstance().getEpochDuration() * Simulator::getInstance().getMaxFiringRate()));
   int idxSp = (spikeCount_[index] + spikeCountOffset_[index] + maxSpikes + offIndex) % maxSpikes;
   return spikeHistory_[index][idxSp];
}

#endif
