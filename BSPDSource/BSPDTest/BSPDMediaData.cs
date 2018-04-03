using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BSPDTest
{
    enum BSPDMediaDataType
    {
        Audio,
        Video
    }

    class BSPDMediaData
    {
        /// <summary>
        /// Y U V Data
        /// </summary>
        public byte[] YData { get; set; }
        public byte[] UData { get; set; }
        public byte[] VData { get; set; }
        /// <summary>
        /// Audio Data
        /// </summary>
        public byte[] AData { get; set; }

        public long vpts { get; set; }
        public long apts { get; set; }
        public long vdur { get; set; }
        public long adur { get; set; }

        public BSPDMediaDataType MediaType { get; set; }

        public bool NeedUpdate { get; set; }

    }
}
