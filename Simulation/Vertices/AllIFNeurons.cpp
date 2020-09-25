#include "AllIFNeurons.h"
#include "ParseParamError.h"
#include "Layout.h"
#include "RNG/MersenneTwister.h"
#include "RNG/Norm.h"
#include "ParameterManager.h"
#include "OperationManager.h"

// Default constructor
AllIFNeurons::AllIFNeurons() {
   C1_ = NULL;
   C2_ = NULL;
   Cm_ = NULL;
   I0_ = NULL;
   Iinject_ = NULL;
   Inoise_ = NULL;
   Isyn_ = NULL;
   Rm_ = NULL;
   Tau_ = NULL;
   Trefract_ = NULL;
   Vinit_ = NULL;
   Vm_ = NULL;
   Vreset_ = NULL;
   Vrest_ = NULL;
   Vthresh_ = NULL;
   numStepsInRefractoryPeriod_ = NULL;
}

AllIFNeurons::~AllIFNeurons() {
   freeResources();
}

/*
 *  Setup the internal structure of the class (allocate memories).
 */
void AllIFNeurons::setupNeurons() {
   AllSpikingNeurons::setupNeurons();

   // TODO: Rename variables for easier identification
   C1_ = new BGFLOAT[size_];
   C2_ = new BGFLOAT[size_];
   Cm_ = new BGFLOAT[size_];
   I0_ = new BGFLOAT[size_];
   Iinject_ = new BGFLOAT[size_];
   Inoise_ = new BGFLOAT[size_];
   Isyn_ = new BGFLOAT[size_];
   Rm_ = new BGFLOAT[size_];
   Tau_ = new BGFLOAT[size_];
   Trefract_ = new BGFLOAT[size_];
   Vinit_ = new BGFLOAT[size_];
   Vm_ = new BGFLOAT[size_];
   Vreset_ = new BGFLOAT[size_];
   Vrest_ = new BGFLOAT[size_];
   Vthresh_ = new BGFLOAT[size_];
   numStepsInRefractoryPeriod_ = new int[size_];

   for (int i = 0; i < size_; ++i) {
      numStepsInRefractoryPeriod_[i] = 0;
   }
}

/*
 *  Cleanup the class (deallocate memories).
 */
void AllIFNeurons::cleanupNeurons() {
   freeResources();
   AllSpikingNeurons::cleanupNeurons();
}

/**
 *  Deallocate all resources
 */
void AllIFNeurons::freeResources() {
   if (size_ != 0) {
      delete[] C1_;
      delete[] C2_;
      delete[] Cm_;
      delete[] I0_;
      delete[] Iinject_;
      delete[] Inoise_;
      delete[] Isyn_;
      delete[] Rm_;
      delete[] Tau_;
      delete[] Trefract_;
      delete[] Vinit_;
      delete[] Vm_;
      delete[] Vreset_;
      delete[] Vrest_;
      delete[] Vthresh_;
      delete[] numStepsInRefractoryPeriod_;
   }

   C1_ = NULL;
   C2_ = NULL;
   Cm_ = NULL;
   I0_ = NULL;
   Iinject_ = NULL;
   Inoise_ = NULL;
   Isyn_ = NULL;
   Rm_ = NULL;
   Tau_ = NULL;
   Trefract_ = NULL;
   Vinit_ = NULL;
   Vm_ = NULL;
   Vreset_ = NULL;
   Vrest_ = NULL;
   Vthresh_ = NULL;
   numStepsInRefractoryPeriod_ = NULL;
}

/**
 *  Load member variables from configuration file.
 *  Registered to OperationManager as Operation::op::loadParameters
 */
void AllIFNeurons::loadParameters() {
   ParameterManager::getInstance().getBGFloatByXpath("//Iinject/min/text()", IinjectRange_[0]);
   ParameterManager::getInstance().getBGFloatByXpath("//Iinject/max/text()", IinjectRange_[1]);

   ParameterManager::getInstance().getBGFloatByXpath("//Inoise/min/text()", InoiseRange_[0]);
   ParameterManager::getInstance().getBGFloatByXpath("//Inoise/max/text()", InoiseRange_[1]);

   ParameterManager::getInstance().getBGFloatByXpath("//Vthresh/min/text()", VthreshRange_[0]);
   ParameterManager::getInstance().getBGFloatByXpath("//Vthresh/max/text()", VthreshRange_[1]);

   ParameterManager::getInstance().getBGFloatByXpath("//Vresting/min/text()", VrestingRange_[0]);
   ParameterManager::getInstance().getBGFloatByXpath("//Vresting/max/text()", VrestingRange_[1]);

   ParameterManager::getInstance().getBGFloatByXpath("//Vreset/min/text()", VresetRange_[0]);
   ParameterManager::getInstance().getBGFloatByXpath("//Vreset/max/text()", VresetRange_[1]);

   ParameterManager::getInstance().getBGFloatByXpath("//Vinit/min/text()", VinitRange_[0]);
   ParameterManager::getInstance().getBGFloatByXpath("//Vinit/max/text()", VinitRange_[1]);

   ParameterManager::getInstance().getBGFloatByXpath("//starter_vthresh/min/text()", starterVthreshRange_[0]);
   ParameterManager::getInstance().getBGFloatByXpath("//starter_vthresh/max/text()", starterVthreshRange_[1]);

   ParameterManager::getInstance().getBGFloatByXpath("//starter_vreset/min/text()", starterVresetRange_[0]);
   ParameterManager::getInstance().getBGFloatByXpath("//starter_vreset/max/text()", starterVresetRange_[1]);
}

/**
 *  Prints out all parameters of the neurons to logging file.
 *  Registered to OperationManager as Operation::printParameters
 */
void AllIFNeurons::printParameters() const {
   AllNeurons::printParameters();

   LOG4CPLUS_DEBUG(fileLogger_,
                   "\n\tInterval of constant injected current: ["
                         << IinjectRange_[0] << ", " << IinjectRange_[1] << "]"
                         << endl
                         << "\tInterval of STD of (gaussian) noise current: ["
                         << InoiseRange_[0] << ", " << InoiseRange_[1] << "]"
                         << endl
                         << "\tInterval of firing threshold: ["
                         << VthreshRange_[0] << ", " << VthreshRange_[1] << "]"
                         << endl
                         << "\tInterval of asymptotic voltage (Vresting): [" << VrestingRange_[0]
                         << ", " << VrestingRange_[1] << "]"
                         << endl
                         << "\tInterval of reset voltage: [" << VresetRange_[0]
                         << ", " << VresetRange_[1] << "]"
                         << endl
                         << "\tInterval of initial membrance voltage: [" << VinitRange_[0]
                         << ", " << VinitRange_[1] << "]"
                         << endl
                         << "\tStarter firing threshold: [" << starterVthreshRange_[0]
                         << ", " << starterVthreshRange_[1] << "]"
                         << endl
                         << "\tStarter reset threshold: [" << starterVresetRange_[0]
                         << ", " << starterVresetRange_[1] << "]"
                         << endl << endl);
}

/*
 *  Creates all the Neurons and generates data for them.
 *
 *  @param  layout      Layout information of the neunal network.
 */
void AllIFNeurons::createAllNeurons(Layout *layout) {
   /* set their specific types */
   for (int neuron_index = 0; neuron_index < Simulator::getInstance().getTotalNeurons(); neuron_index++) {
      setNeuronDefaults(neuron_index);

      // set the neuron info for neurons
      createNeuron(neuron_index, layout);
   }
}

/*
 *  Creates a single Neuron and generates data for it.
 *
 *  @param  neuronIndex Index of the neuron to create.
 *  @param  layout       Layout information of the neunal network.
 */
void AllIFNeurons::createNeuron(int neuronIndex, Layout *layout) {
   // set the neuron info for neurons
   Iinject_[neuronIndex] = rng.inRange(IinjectRange_[0], IinjectRange_[1]);
   Inoise_[neuronIndex] = rng.inRange(InoiseRange_[0], InoiseRange_[1]);
   Vthresh_[neuronIndex] = rng.inRange(VthreshRange_[0], VthreshRange_[1]);
   Vrest_[neuronIndex] = rng.inRange(VrestingRange_[0], VrestingRange_[1]);
   Vreset_[neuronIndex] = rng.inRange(VresetRange_[0], VresetRange_[1]);
   Vinit_[neuronIndex] = rng.inRange(VinitRange_[0], VinitRange_[1]);
   Vm_[neuronIndex] = Vinit_[neuronIndex];

   initNeuronConstsFromParamValues(neuronIndex, Simulator::getInstance().getDeltaT());

   int maxSpikes = (int) ((Simulator::getInstance().getEpochDuration() * Simulator::getInstance().getMaxFiringRate()));
   spikeHistory_[neuronIndex] = new uint64_t[maxSpikes];
   for (int j = 0; j < maxSpikes; ++j) {
      spikeHistory_[neuronIndex][j] = ULONG_MAX;
   }

   switch (layout->neuronTypeMap_[neuronIndex]) {
      case INH:
         LOG4CPLUS_DEBUG(neuronLogger_, "Setting inhibitory neuron: " << neuronIndex);
         // set inhibitory absolute refractory period
         Trefract_[neuronIndex] = DEFAULT_InhibTrefract;// TODO(derek): move defaults inside model.
         break;

      case EXC:
         LOG4CPLUS_DEBUG(neuronLogger_, "Setting excitatory neuron: " << neuronIndex);
         // set excitatory absolute refractory period
         Trefract_[neuronIndex] = DEFAULT_ExcitTrefract;
         break;

      default:
         LOG4CPLUS_DEBUG(neuronLogger_, "ERROR: unknown neuron type: "
               << layout->neuronTypeMap_[neuronIndex] << "@" << neuronIndex);
         assert(false);
         break;
   }
   // endogenously_active_neuron_map -> Model State
   if (layout->starterMap_[neuronIndex]) {
      // set endogenously active threshold voltage, reset voltage, and refractory period
      Vthresh_[neuronIndex] = rng.inRange(starterVthreshRange_[0], starterVthreshRange_[1]);
      Vreset_[neuronIndex] = rng.inRange(starterVresetRange_[0], starterVresetRange_[1]);
      Trefract_[neuronIndex] = DEFAULT_ExcitTrefract; // TODO(derek): move defaults inside model.
   }

   LOG4CPLUS_DEBUG(neuronLogger_, "\nCREATE NEURON[" << neuronIndex << "] {" << endl
                 << "\tVm = " << Vm_[neuronIndex] << endl
                 << "\tVthresh = " << Vthresh_[neuronIndex] << endl
                 << "\tI0 = " << I0_[neuronIndex] << endl
                 << "\tInoise = " << Inoise_[neuronIndex] << " from : (" << InoiseRange_[0] << "," << InoiseRange_[1]
                 << ")"
                 << endl
                 << "\tC1 = " << C1_[neuronIndex] << endl
                 << "\tC2 = " << C2_[neuronIndex] << endl
                 << "}" << endl);
}

/*
 *  Set the Neuron at the indexed location to default values.
 *
 *  @param  index    Index of the Neuron that the synapse belongs to.
 */
void AllIFNeurons::setNeuronDefaults(const int index) {
   Cm_[index] = DEFAULT_Cm;
   Rm_[index] = DEFAULT_Rm;
   Vthresh_[index] = DEFAULT_Vthresh;
   Vrest_[index] = DEFAULT_Vrest;
   Vreset_[index] = DEFAULT_Vreset;
   Vinit_[index] = DEFAULT_Vreset;
   Trefract_[index] = DEFAULT_Trefract;
   Inoise_[index] = DEFAULT_Inoise;
   Iinject_[index] = DEFAULT_Iinject;
   Tau_[index] = DEFAULT_Cm * DEFAULT_Rm;
}

/*
 *  Initializes the Neuron constants at the indexed location.
 *
 *  @param  neuronIndex    Index of the Neuron.
 *  @param  deltaT          Inner simulation step duration
 */
void AllIFNeurons::initNeuronConstsFromParamValues(int neuronIndex, const BGFLOAT deltaT) {
   BGFLOAT &Tau = this->Tau_[neuronIndex];
   BGFLOAT &C1 = this->C1_[neuronIndex];
   BGFLOAT &C2 = this->C2_[neuronIndex];
   BGFLOAT &Rm = this->Rm_[neuronIndex];
   BGFLOAT &I0 = this->I0_[neuronIndex];
   BGFLOAT &Iinject = this->Iinject_[neuronIndex];
   BGFLOAT &Vrest = this->Vrest_[neuronIndex];

   /* init consts C1,C2 for exponential Euler integration */
   if (Tau > 0) {
      C1 = exp(-deltaT / Tau);
      C2 = Rm * (1 - C1);
   } else {
      C1 = 0.0;
      C2 = Rm;
   }
   /* calculate const IO */
   if (Rm > 0) {
      I0 = Iinject + Vrest / Rm;
   } else {
      assert(false);
   }
}

/*
 *  Outputs state of the neuron chosen as a string.
 *
 *  @param  index  index of the neuron (in neurons) to output info from.
 *  @return the complete state of the neuron.
 */
string AllIFNeurons::toString(const int index) const {
   stringstream ss;
   ss << "Cm: " << Cm_[index] << " "; // membrane capacitance
   ss << "Rm: " << Rm_[index] << " "; // membrane resistance
   ss << "Vthresh: " << Vthresh_[index] << " "; // if Vm exceeds, Vthresh, a spike is emitted
   ss << "Vrest: " << Vrest_[index] << " "; // the resting membrane voltage
   ss << "Vreset: " << Vreset_[index] << " "; // The voltage to reset Vm to after a spike
   ss << "Vinit: " << Vinit_[index] << endl; // The initial condition for V_m at t=0
   ss << "Trefract: " << Trefract_[index] << " "; // the number of steps in the refractory period
   ss << "Inoise: " << Inoise_[index] << " "; // the stdev of the noise to be added each delta_t
   ss << "Iinject: " << Iinject_[index] << " "; // A constant current to be injected into the LIF neuron
   ss << "nStepsInRefr: " << numStepsInRefractoryPeriod_[index]
      << endl; // the number of steps left in the refractory period
   ss << "Vm: " << Vm_[index] << " "; // the membrane voltage
   ss << "hasFired: " << hasFired_[index] << " "; // it done fired?
   ss << "C1: " << C1_[index] << " ";
   ss << "C2: " << C2_[index] << " ";
   ss << "I0: " << I0_[index] << " ";
   return ss.str();
}

/*
 *  Sets the data for Neurons to input's data.
 *
 *  @param  input       istream to read from.
 */
void AllIFNeurons::deserialize(istream &input) {
   for (int i = 0; i < Simulator::getInstance().getTotalNeurons(); i++) {
      readNeuron(input, i);
   }
}

/*
 *  Sets the data for Neuron #index to input's data.
 *
 *  @param  input       istream to read from.
 *  @param  i           index of the neuron (in neurons).
 */
void AllIFNeurons::readNeuron(istream &input, int i) {
   // input.ignore() so input skips over end-of-line characters.
   input >> Cm_[i];
   input.ignore();
   input >> Rm_[i];
   input.ignore();
   input >> Vthresh_[i];
   input.ignore();
   input >> Vrest_[i];
   input.ignore();
   input >> Vreset_[i];
   input.ignore();
   input >> Vinit_[i];
   input.ignore();
   input >> Trefract_[i];
   input.ignore();
   input >> Inoise_[i];
   input.ignore();
   input >> Iinject_[i];
   input.ignore();
   input >> Isyn_[i];
   input.ignore();
   input >> numStepsInRefractoryPeriod_[i];
   input.ignore();
   input >> C1_[i];
   input.ignore();
   input >> C2_[i];
   input.ignore();
   input >> I0_[i];
   input.ignore();
   input >> Vm_[i];
   input.ignore();
   input >> hasFired_[i];
   input.ignore();
   input >> Tau_[i];
   input.ignore();
}

/*
 *  Writes out the data in Neurons.
 *
 *  @param  output      stream to write out to.
 */
void AllIFNeurons::serialize(ostream &output) const {
   for (int i = 0; i < Simulator::getInstance().getTotalNeurons(); i++) {
      writeNeuron(output, i);
   }
}

/*
 *  Writes out the data in the selected Neuron.
 *
 *  @param  output      stream to write out to.
 *  @param  i           index of the neuron (in neurons).
 */
void AllIFNeurons::writeNeuron(ostream &output, int i) const {
   output << Cm_[i] << ends;
   output << Rm_[i] << ends;
   output << Vthresh_[i] << ends;
   output << Vrest_[i] << ends;
   output << Vreset_[i] << ends;
   output << Vinit_[i] << ends;
   output << Trefract_[i] << ends;
   output << Inoise_[i] << ends;
   output << Iinject_[i] << ends;
   output << Isyn_[i] << ends;
   output << numStepsInRefractoryPeriod_[i] << ends;
   output << C1_[i] << ends;
   output << C2_[i] << ends;
   output << I0_[i] << ends;
   output << Vm_[i] << ends;
   output << hasFired_[i] << ends;
   output << Tau_[i] << ends;
}


