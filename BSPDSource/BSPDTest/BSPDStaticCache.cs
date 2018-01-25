using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BSPDTest
{
    class BSPDStaticCache
    {
        private int cacheNum;
        private int windex;
        private int rindex;
        private int size;
        public BSPDMediaData[] mediaQueue;
        object rwLocker = new object();
        public BSPDStaticCache(int cache)
        {
            cacheNum = cache;
            InitCache();
        }

        public BSPDStaticCache(int sec,int fps)
        {
            cacheNum = sec * fps;
            InitCache();
        }

        public BSPDStaticCache()
        {
            cacheNum = 10;
            InitCache();
        }

        private void InitCache()
        {
            mediaQueue = new BSPDMediaData[cacheNum];
            BSPDMediaData data;
            windex = 0;
            rindex = 0;
            size = 0;
            //for (int i = 0; i < cacheNum; i++)
            //{
            //    data = new BSPDMediaData();
            //    mediaQueue[i] = data;
            //}
        }

        public int PushIndex()
        {
            if (size >= cacheNum)
            {
                return -1;
            }

            return windex;
        }

        public int PopIndex()
        {
            if (size<0)
            {
                return -1;
            }
            return rindex;
        }


        public bool Push()
        {
            
            if (++windex == cacheNum)
            {
                windex = 0;
            }
            lock (rwLocker)
            {
                size++;
            }
            return true;
        }

        public bool Pop()
        {
            if (++rindex == cacheNum)
            {
                rindex = 0;
            }
            lock (rwLocker)
            {
                size--;
            }
            return true;
        }

    }
}
