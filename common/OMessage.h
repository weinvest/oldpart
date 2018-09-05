#ifndef _OLDPARD_OMESSAGE_H
#define _OLDPARD_OMESSAGE_H


struct OMessage
{
    int32_t length;
    union {
        int32_t version;
        struct {
            int16_t major;
            int16_t minor；
        };
    };
    int32_t sequenceId;
    int32_t messageId; //meesage type
    int32_t messageSequenceId; //一个大的Message可以被分成多个小的OMessage, 这样的OMessage从０开始编号, <0表示最后一个OMessage
};

#endif /* end of include guard: _OLDPARD_OMESSAGE_H */
