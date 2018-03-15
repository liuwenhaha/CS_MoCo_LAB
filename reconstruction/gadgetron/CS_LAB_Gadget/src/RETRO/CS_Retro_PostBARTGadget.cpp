#include "CS_Retro_PostBARTGadget.h"

#include <mri_core_data.h>

using namespace Gadgetron;

// class constructor
CS_Retro_PostBARTGadget::CS_Retro_PostBARTGadget()
{
}

// class destructor
CS_Retro_PostBARTGadget::~CS_Retro_PostBARTGadget()
{
}

int CS_Retro_PostBARTGadget::process_config(ACE_Message_Block *mb)
{
	return GADGET_OK;
}

int CS_Retro_PostBARTGadget::process(GadgetContainerMessage<IsmrmrdImageArray> *m1)
{
	// get data
	hoNDArray<std::complex<float> > data = m1->getObjectPtr()->data_;

	// permute array: kx-ky-kz-c-t-s-slc -> kx-ky-kz-t-c-s-slc (with c=s=slc=1 - will be cropped later on)
	std::vector<size_t> vtDimOrder;
	vtDimOrder.push_back(0);
	vtDimOrder.push_back(1);
	vtDimOrder.push_back(2);
	vtDimOrder.push_back(4);
	vtDimOrder.push_back(3);
	vtDimOrder.push_back(5);
	vtDimOrder.push_back(6);
	data = *permute(&data, &vtDimOrder, false);
	
	// crop array and convert to GadgetContainerMessage
	GadgetContainerMessage<hoNDArray<std::complex<float> > > *cm2 = new GadgetContainerMessage<hoNDArray<std::complex<float> > >();
	cm2->getObjectPtr()->create(data.get_size(0), data.get_size(1), data.get_size(2), data.get_size(3));
	memcpy(cm2->getObjectPtr()->get_data_ptr(), data.get_data_ptr(), sizeof(std::complex<float>)*data.get_size(0)*data.get_size(1)*data.get_size(2)*data.get_size(3));

	// create and set ImageHeader
	GadgetContainerMessage<ISMRMRD::ImageHeader> *cm1 = new GadgetContainerMessage<ISMRMRD::ImageHeader>();
	fCopyImageHeader(cm1, &m1->getObjectPtr()->headers_.at(0));

	// reset number of gates (otherwise value is lost and no output is performed)
	cm1->getObjectPtr()->channels = GlobalVar::instance()->iNPhases_;

	// concatenate data
	cm1->cont(cm2);

	// put data on pipeline
	if (this->next()->putq(cm1) < 0) {
		return GADGET_FAIL;
	}

	return GADGET_OK;
}

GADGET_FACTORY_DECLARE(CS_Retro_PostBARTGadget)
