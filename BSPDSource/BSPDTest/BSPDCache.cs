using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;


namespace BSPDTest
{

    class BSPDCache
    {
        public Queue<BSPDMediaData> AQueue = new Queue<BSPDMediaData>();
        public Queue<BSPDMediaData> VQueue = new Queue<BSPDMediaData>();

        object alock = new object();
        object vlock = new object();

        public bool Push(BSPDMediaData data)
        {
            if (data.MediaType == BSPDMediaDataType.Audio)
            {
                lock (alock)
                {
                    if (AQueue.Count<10)
                    {
                        AQueue.Enqueue(data);
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
            else
            {
                lock (vlock)
                {
                    if (VQueue.Count<10)
                    {
                        VQueue.Enqueue(data);
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }
            }
        }

        public BSPDMediaData Pop(BSPDMediaDataType mediaDataType)
        {
            if (mediaDataType == BSPDMediaDataType.Audio)
            {
                lock (alock)
                {
                    if (AQueue.Count>0)
                    {
                        return AQueue.Dequeue();
                    }
                    else
                    {
                        return null;
                    }
                }
            }
            else
            {
                lock (vlock)
                {
                    if (VQueue.Count>0)
                    {
                        return VQueue.Dequeue();
                    }
                    else
                    {
                        return null;
                    }
                }
            }
        }

        object apktLock = new object();
        object vpktLock = new object();
        public Queue<IntPtr> APktQueue = new Queue<IntPtr>();
        public Queue<IntPtr> vPktQueue = new Queue<IntPtr>();


        public bool Push(IntPtr data)
        {
            throw new NotImplementedException();
        }

        public bool Pop(IntPtr data)
        {
            throw new NotImplementedException();
        }

    }
}
