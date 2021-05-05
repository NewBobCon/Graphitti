/*
 *      \file SInputPoisson.cpp
 *
 *      \author Fumitaka Kawasaki
 *
 *      \brief A class that performs stimulus input (implementation Poisson).
 */

#include "SInputPoisson.h"
#include "tinyxml.h"
#include "AllDSSynapses.h"

extern void getValueList(const string& valString, vector<BGFLOAT>* pList);

/*
 * constructor
 * @param[in] parms     Pointer to xml parms element
 */
SInputPoisson::SInputPoisson(TiXmlElement* parms) :
    nISIs(NULL),
    synapses_(NULL),
    masks(NULL)
{
    fSInput = false;

    // read fr_mean and weight
    TiXmlElement* temp = NULL;
    string sync;
    BGFLOAT fr_mean;	// firing rate (per sec)

    if (( temp = parms->FirstChildElement( "IntParams" ) ) != NULL) 
    { 
        if (temp->QueryFLOATAttribute("fr_mean", &fr_mean ) != TIXML_SUCCESS) {
            cerr << "error IntParams:fr_mean" << endl;
            return;
        }
        if (temp->QueryFLOATAttribute("weight", &weight ) != TIXML_SUCCESS) {
            cerr << "error IntParams:weight" << endl;
            return;
        }
    }
    else
    {
        cerr << "missing IntParams" << endl;
        return;
    }

     // initialize firng rate, inverse firing rate
    fr_mean = fr_mean / 1000;	// firing rate per msec
    lambda = 1 / fr_mean;	// inverse firing rate

    // allocate memory for interval counter
    nISIs = new int[Simulator::getInstance().getTotalNeurons()];
    memset(nISIs, 0, sizeof(int) * Simulator::getInstance().getTotalNeurons());
    
    // allocate memory for input masks
    masks = new bool[Simulator::getInstance().getTotalNeurons()];

    // read mask values and set it to masks
    vector<BGFLOAT> maskIndex;
    if ((temp = parms->FirstChildElement( "Masks")) != NULL)
    {
       TiXmlNode* pNode = NULL;
        while ((pNode = temp->IterateChildren(pNode)) != NULL)
        {
            if (strcmp(pNode->Value(), "M") == 0)
            {
                getValueList(pNode->ToElement()->GetText(), &maskIndex);

                memset(masks, false, sizeof(bool) * Simulator::getInstance().getTotalNeurons());
                for (uint32_t i = 0; i < maskIndex.size(); i++)
                    masks[static_cast<int> ( maskIndex[i] )] = true;
            }
            else if (strcmp(pNode->Value(), "LayoutFiles") == 0)
            {
                string maskNListFileName;

                if (pNode->ToElement()->QueryValueAttribute( "maskNListFileName", &maskNListFileName ) == TIXML_SUCCESS)
                {
                    TiXmlDocument simDoc( maskNListFileName.c_str( ) );
                    if (!simDoc.LoadFile( ))
                    {
                        cerr << "Failed loading positions of stimulus input mask neurons list file " << maskNListFileName << ":" << "\n\t"
                             << simDoc.ErrorDesc( ) << endl;
                        cerr << " error: " << simDoc.ErrorRow( ) << ", " << simDoc.ErrorCol( ) << endl;
                        break;
                    }
                    TiXmlNode* temp2 = NULL;
                    if (( temp2 = simDoc.FirstChildElement( "M" ) ) == NULL)
                    {
                        cerr << "Could not find <M> in positons of stimulus input mask neurons list file " << maskNListFileName << endl;
                        break;
                    }
                    getValueList(temp2->ToElement()->GetText(), &maskIndex);

                    memset(masks, false, sizeof(bool) * Simulator::getInstance().getTotalNeurons());
                    for (uint32_t i = 0; i < maskIndex.size(); i++)
                        masks[static_cast<int> ( maskIndex[i] )] = true;
                }
            }
        }
    }
    else
    {
        // when no mask is specified, set it all true
        memset(masks, true, sizeof(bool) * Simulator::getInstance().getTotalNeurons());
    }

    fSInput = true;
}

/*
 * destructor
 */
SInputPoisson::~SInputPoisson()
{
}

/*
 * Initialize data.
 *
 *  @param[in] psi       Pointer to the simulation information.
 */
void SInputPoisson::init()
{
    if (fSInput == false)
        return;

    // create an input synapse layer
    // TODO: do we need to support other types of synapses?
    synapses_ = new AllDSSynapses(Simulator::getInstance().getTotalNeurons(), 1);
    for (int neuronIndex = 0; neuronIndex < Simulator::getInstance().getTotalNeurons(); neuronIndex++)
    {
        synapseType type;
        if (Simulator::getInstance().getModel()->getLayout()->neuronTypeMap_[neuronIndex] == INH)
            type = EI;
        else
            type = EE;

        BGFLOAT* sumPoint = &(Simulator::getInstance().getPSummationMap()[neuronIndex]);
        BGSIZE iSyn = Simulator::getInstance().getMaxSynapsesPerNeuron() * neuronIndex;

        synapses_->createSynapse(iSyn, 0, neuronIndex, sumPoint, Simulator::getInstance().getDeltaT(), type);
        dynamic_cast<AllSynapses*>(synapses_)->W_[iSyn] = weight * AllSynapses::SYNAPSE_STRENGTH_ADJUSTMENT;
    }
}

/*
 * Terminate process.
 *
 *  @param[in] psi       Pointer to the simulation information.
 */
void SInputPoisson::term()
{
    // clear memory for interval counter
    if (nISIs != NULL)
        delete[] nISIs;

    // clear the synapse layer, which destroy all synase objects
    if (synapses_ != NULL)
        delete synapses_;

    // clear memory for input masks
    if (masks != NULL)
        delete[] masks;
}
