#ifndef __HN_DATA_H__
#define __HN_DATA_H__

typedef long ssize_t;

class Data final
{
public:
	static const Data Null;

	Data();
	Data(const Data& other);
	Data(Data&& other);
	~Data();

	// Assignment operator
	Data& operator= (const Data& other);
	Data& operator= (Data&& other);

	/**
	* @js NA
	* @lua NA
	*/
	unsigned char* getBytes() const;
	/**
	* @js NA
	* @lua NA
	*/
	ssize_t getSize() const;

	/** Copies the buffer pointer and its size.
	*  @note This method will copy the whole buffer.
	*        Developer should free the pointer after invoking this method.
	*  @see Data::fastSet
	*/
	void copy(const unsigned char* bytes, const ssize_t size);

	/** Fast set the buffer pointer and its size. Please use it carefully.
	*  @param bytes The buffer pointer, note that it have to be allocated by 'malloc' or 'calloc',
	*         since in the destructor of Data, the buffer will be deleted by 'free'.
	*  @note 1. This method will move the ownship of 'bytes'pointer to Data,
	*        2. The pointer should not be used outside after it was passed to this method.
	*  @see Data::copy
	*/
	void fastSet(unsigned char* bytes, const ssize_t size);

	/** Clears data, free buffer and reset data size */
	void clear();

	/** Check whether the data is null. */
	bool isNull() const;

private:
	void move(Data& other);

private:
	unsigned char* _bytes;
	ssize_t _size;
};


#endif // __HN_DATA_H__
