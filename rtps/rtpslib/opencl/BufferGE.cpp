//#include "BufferGE.h"

//namespace rtps {

template <class T>
BufferGE<T>::BufferGE(CL *cli, T* data, int sz)
{
    this->cli = cli;
    this->data = data;
	this->nb_el = sz;

	if (data) {
		externalPtr = true;

		// create buffer on GPU
    	cl_buffer.push_back(cl::Buffer(cli->context, CL_MEM_READ_WRITE, sz*sizeof(T), NULL, &cli->err));
    	//copyToDevice();
	}
}

//----------------------------------------------------------------------
template <class T>
BufferGE<T>::BufferGE(CL *cli, int sz)
{
    this->cli = cli;
    this->data = data;
	this->nb_el = sz;
	externalPtr = false;
	data = new T [nb_el];

	// create buffer on GPU
   	cl_buffer.push_back(cl::Buffer(cli->context, CL_MEM_READ_WRITE, sz*sizeof(T), NULL, &cli->err));
    //copyToDevice();
}
//----------------------------------------------------------------------
template <class T>
BufferGE<T>::~BufferGE()
{
	if (externalPtr == false && data) {
		delete [] data;
		data = 0;
	}
}

//----------------------------------------------------------------------
template <class T>
void BufferGE<T>::copyToDevice()
{
    //TODO clean up this memory/buffer issue (nasty pointer casting)
	if (data) {
    	cli->err = cli->queue.enqueueWriteBuffer(*((cl::Buffer*)&cl_buffer[0]), CL_TRUE, 0, nb_el*sizeof(T), data, NULL, &cli->event);
    	cli->queue.finish();
	}

}

//----------------------------------------------------------------------
template <class T>
void BufferGE<T>::copyToHost()
{
    //TODO clean up this memory/buffer issue
    //std::vector<T> data(num);
    //TODO pass back a pointer instead of a copy
    //std::vector<T> data = new std::vector<T>(num);

	if (data) {
    	cli->err = cli->queue.enqueueReadBuffer(*((cl::Buffer*)&cl_buffer[0]), CL_TRUE, 0, nb_el*sizeof(T), data, NULL, &cli->event);
    	cli->queue.finish();
	}
}

//----------------------------------------------------------------------

