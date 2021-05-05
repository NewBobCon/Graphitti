/**
 @file GPUSpikingModel

 @brief Implementation of Model for the spiking neural networks.

 The Model class maintains and manages classes of objects that make up
 essential components of the spiking neural networks.
    -# IAllNeurons: A class to define a list of particular type of neurons.
    -# IAllSynapses: A class to define a list of particular type of synapses.
    -# Connections: A class to define connections of the neural network.
    -# Layout: A class to define neurons' layout information in the network.

 The network is composed of 3 superimposed 2-d arrays: neurons, synapses, and
 summation points.

 Synapses in the synapse map are located at the coordinates of the neuron
 from which they receive output.  Each synapse stores a pointer into a
 summation point.

 If, during an advance cycle, a neuron \f$A\f$ at coordinates \f$x,y\f$ fires, every synapse
 which receives output is notified of the spike. Those synapses then hold
 the spike until their delay period is completed.  At a later advance cycle, once the delay
 period has been completed, the synapses apply their PSRs (Post-Synaptic-Response) to
 the summation points.

 Finally, on the next advance cycle, each neuron \f$B\f$ adds the value stored
 in their corresponding summation points to their \f$V_m\f$ and resets the summation points to
 zero.

 The model runs on multi-threaded on a GPU.

 */

#pragma once

#include "AllSpikingNeurons.h"
#include "AllSpikingSynapses.h"

#ifdef __CUDACC__
#include "Book.h"
#endif

const BGFLOAT SYNAPSE_STRENGTH_ADJUSTMENT = 1.0e-8;

/************************************************
 *   Inline Functions for handling
 *   performance recording
 ***********************************************/

#if defined(PERFORMANCE_METRICS) && defined(__CUDACC__)
extern float g_time;
extern cudaEvent_t start, stop;

inline void cudaStartTimer() {
          cudaEventRecord( start, 0 );
};

//*! Increment elapsed time in seconds
inline void cudaLapTime(double& t_event) {
          cudaEventRecord( stop, 0 );
          cudaEventSynchronize( stop );
          cudaEventElapsedTime( &g_time, start, stop );
   // The CUDA functions return time in milliseconds
          t_event += g_time/1000.0;
};
#endif // PERFORMANCE_METRICS

class AllSpikingSynapses;

class GPUSpikingModel : public Model {
   friend class GpuSInputPoisson;

public:
   GPUSpikingModel();

   virtual ~GPUSpikingModel();

   /// Set up model state, if anym for a specific simulation run.
   virtual void setupSim();

   /// Performs any finalization tasks on network following a simulation.
   virtual void finish();

   /// Advances network state one simulation step.
   virtual void advance();

   /// Modifies connections between neurons based on current state of the network and behavior
   /// over the past epoch. Should be called once every epoch.
   virtual void updateConnections();

   /// Copy GPU Synapse data to CPU.
   virtual void copyGPUtoCPU();

   /// Copy CPU Synapse data to GPU.
   virtual void copyCPUtoGPU();

   /// Print out SynapseProps on the GPU.
   void printGPUSynapsesPropsModel() const;

protected:
   /// Allocates  and initializes memories on CUDA device.
   /// @param[out] allNeuronsDevice          Memory location of the pointer to the neurons list on device memory.
   /// @param[out] allSynapsesDevice         Memory location of the pointer to the synapses list on device memory.
   void allocDeviceStruct(void **allNeuronsDevice, void **allSynapsesDevice);

   /// Copies device memories to host memories and deallocates them.
   /// @param[out] allNeuronsDevice          Memory location of the pointer to the neurons list on device memory.
   /// @param[out] allSynapsesDevice         Memory location of the pointer to the synapses list on device memory.
   virtual void deleteDeviceStruct(void **allNeuronsDevice, void **allSynapsesDevice);

   /// Add psr of all incoming synapses to summation points.
   virtual void calcSummationMap();

   /// Pointer to device random noise array.
   float *randNoise_d;

   /// Pointer to synapse index map in device memory.
   SynapseIndexMap *synapseIndexMapDevice_;

   /// Synapse structures in device memory.
   AllSpikingSynapsesDeviceProperties *allSynapsesDevice_;

   /// Neuron structure in device memory.
   AllSpikingNeuronsDeviceProperties *allNeuronsDevice_;

private:
   void allocSynapseImap(int count);

   void deleteSynapseImap();

public: //2020/03/14 changed to public for accessing in BGDriver

   void copySynapseIndexMapHostToDevice(SynapseIndexMap &synapseIndexMapHost, int numNeurons);

private:

   void updateHistory();

   // TODO
   void eraseSynapse(IAllSynapses &synapses, const int neuronIndex, const int synapseIndex);

   // TODO
   void addSynapse(IAllSynapses &synapses, synapseType type, const int srcNeuron, const int destNeuron,
                   Coordinate &source, Coordinate &dest, BGFLOAT *sumPoint, BGFLOAT deltaT);

   // TODO
   void createSynapse(IAllSynapses &synapses, const int neuronIndex, const int synapseIndex,
                      Coordinate source, Coordinate dest, BGFLOAT *sp, BGFLOAT deltaT, synapseType type);
};

#if defined(__CUDACC__)
extern "C" {
void normalMTGPU(float * randNoise_d);
void initMTGPU(unsigned int seed, unsigned int blocks, unsigned int threads, unsigned int nPerRng, unsigned int mt_rng_count); 
}       
        
//! Calculate summation point.
extern __global__ void calcSummationMapDevice(int totalNeurons,
          AllSpikingNeuronsDeviceProperties* __restrict__ allNeurnsDevice,
          const SynapseIndexMap* __restrict__ synapseIndexMapDevice_,
                    const AllSpikingSynapsesDeviceProperties* __restrict__ allSynapsesDevice );
#endif
