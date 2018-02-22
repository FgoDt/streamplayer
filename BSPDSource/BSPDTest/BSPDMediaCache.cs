using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BSPDTest
{
    enum BSPDSkipMode
    {
        drop,
        fast,
    }
    class BSPDMediaCache
    {
        public Queue<BSPDMediaData> queue = new Queue<BSPDMediaData>();

        object qlock = new object();

        private bool ok;
        public bool OK {
            get {
                lock (qlock)
                {
                    if (queue.Count>15&&!ok)
                    {
                        ok = true;
                    }
                    
                }
              return ok;
            }
        }

        public bool Push(BSPDMediaData data)
        {
            lock (qlock)
            {
                if (queue.Count<30)
                {
                    queue.Enqueue(data);
                    return true;
                }
            }

            return false;
        }

        public BSPDMediaData Pop()
        {
            lock (qlock)
            {
                if (queue.Count>0&&OK)
                {
                    return queue.Dequeue();
                }
            }
            ok = false;
            return null;
        }

        public bool TrySkipFrame(int num)
        {
            lock (qlock)
            {
                if (num == -1)
                {
                    num = queue.Count - 1;
                }
                if (queue.Count<num)
                {
                    return false;
                }
                for (int i = 0; i < num; i++)
                {
                    queue.Dequeue();
                }
                return true;
            }

        }



    }
}
