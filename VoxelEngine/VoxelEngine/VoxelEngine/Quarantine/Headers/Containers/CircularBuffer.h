/*HEADER_GOES_HERE*/
#ifndef CircularBuffer_Guard
#define CircularBuffer_Guard

template <typename T, unsigned count>
class circular_buffer
{
public:
  T& operator[](unsigned index)
  {
    index += currentFront;
    if (index >= count)
      index -= count;
    return buffer[index];
  }
  void push_front(const T& value)
  {
    if (--currentFront >= count)
      currentFront = count - 1;
    buffer[currentFront] = value;
  }
private:
  T buffer[count];
  unsigned currentFront = 0;
};
#endif // !CircularBuffer_Guard