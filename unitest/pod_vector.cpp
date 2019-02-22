
#include "test.h"
#include "src/include/data_vector.h"
#include "timeuse.h"

using namespace picasso;

struct data_test 
{
    int i;
    float f;
    double d;
};

volatile int tmp;
static int dummy[4096000];
static void clear_cache(void)
{
    int sum = 0;
    for(int i=0; i<4096000; i++)
        dummy[i] = 2;
    for(int i=0; i<4096000; i++)
        sum += dummy[i];

    tmp = sum;
}

TEST(Pod_Vector, CreateAndDestroy)
{
    pod_vector<unsigned int> iv;

    pod_vector<unsigned int> sv = iv;

    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(0, (int)iv.capacity());
    
    EXPECT_EQ(0, (int)sv.size());
    EXPECT_EQ(0, (int)sv.capacity());

    EXPECT_NE(&iv , &sv);

    pod_vector<data_test> dv(10);

    EXPECT_EQ(0, (int)dv.size());
    EXPECT_EQ(10, (int)dv.capacity());
}

TEST(Pod_Vector, PushAndInsert)
{
    pod_vector<unsigned int> iv;
    pod_vector<unsigned int> sv;

    iv.resize(3);
    sv.resize(5);

    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(3, (int)iv.capacity());
    
    EXPECT_EQ(0, (int)sv.size());
    EXPECT_EQ(5, (int)sv.capacity());

    
    bool b;
    b = iv.push_back(10);
    EXPECT_EQ(true, b);

    b = iv.push_back(11);
    EXPECT_EQ(true, b);

    b = iv.push_back(12);
    EXPECT_EQ(true, b);

    b = iv.push_back(13);
    EXPECT_EQ(false, b);


    for(unsigned int i = 0; i < iv.size(); i++)
        printf("iv: integer vector[%d] = %d\n",i, iv[i]);

    printf("copy vector iv to sv\n");
    sv = iv;

    EXPECT_EQ(3, (int)sv.size());
    EXPECT_EQ(5, (int)sv.capacity());

    EXPECT_EQ(true, sv.is_full());
    EXPECT_EQ(true, iv.is_full());

    for(unsigned int i = 0; i < sv.size(); i++)
        printf("sv: integer vector[%d] = %d\n",i, sv[i]);

    printf("resize iv to 6\n");
    iv.resize(6);
    for(unsigned int i = 0; i < iv.size(); i++) {
        EXPECT_EQ(sv[i], iv[i]);
        printf("iv: integer vector[%d] = %d\n",i, iv[i]);
    }

    EXPECT_EQ(false, iv.is_full());
    EXPECT_EQ(3, (int)iv.size());
    EXPECT_EQ(6, (int)iv.capacity());

    printf("insert value to index 2\n");
    b = iv.insert_at(2, 15);
    EXPECT_EQ(true, b);
    EXPECT_EQ(4, (int)iv.size());
    for(unsigned int i = 0; i < iv.size(); i++)
        printf("iv: integer vector[%d] = %d\n",i, iv[i]);

    printf("insert value to index 15\n");
    b = iv.insert_at(15, 15);
    EXPECT_EQ(false, b);
    EXPECT_EQ(4, (int)iv.size());

    EXPECT_NE(iv.data(), sv.data());

    printf("reset capacity sv\n");
    sv.capacity(1);
    EXPECT_EQ(0, (int)sv.size());
    EXPECT_EQ(5, (int)sv.capacity());

    sv.capacity(10);
    EXPECT_EQ(0, (int)sv.size());
    EXPECT_EQ(10, (int)sv.capacity());

    printf("cut at index 2\n");
    iv.cut_at(2);
    EXPECT_EQ(2, (int)iv.size());
    EXPECT_EQ(6, (int)iv.capacity());
    for(unsigned int i = 0; i < iv.size(); i++)
        printf("iv: integer vector[%d] = %d\n",i, iv[i]);

    printf("set data!\n");
    unsigned int data[] = {100, 200, 300};
    iv.set_data(3, data);
    EXPECT_EQ(3, (int)iv.size());
    EXPECT_EQ(6, (int)iv.capacity());
    for(unsigned int i = 0; i < iv.size(); i++)
        printf("iv: integer vector[%d] = %d\n",i, iv[i]);

    printf("set data!\n");
    unsigned int data2[] = {50, 150, 250, 350, 450, 550 ,650, 750};
    iv.set_data(8, data2);
    EXPECT_EQ(3, (int)iv.size());
    EXPECT_EQ(6, (int)iv.capacity());
    for(unsigned int i = 0; i < iv.size(); i++)
        printf("iv: integer vector[%d] = %d\n",i, iv[i]);

    iv.resize(8);
    iv.set_data(8, data2);
    EXPECT_EQ(8, (int)iv.size());
    EXPECT_EQ(8, (int)iv.capacity());
    for(unsigned int i = 0; i < iv.size(); i++)
        printf("iv: integer vector[%d] = %d\n",i, iv[i]);


    printf("clear iv\n");
    iv.clear();

    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(8, (int)iv.capacity());
}


TEST(Pod_Vector, SpeedCompareWithAgg)
{
    clocktime_t t1, t2;
    double u1, u2;

    pod_vector<unsigned int> pv;
    pod_vector<data_test> pdv;
    data_test d = {1,1,1};

    printf("pod_vector speed testing...\n");
    clear_cache();
    // picasso data vector speed.
    t1 = get_clock();

    pv.resize(10000);
    for(unsigned int i = 0; i < pv.size(); i++)
        pv.push_back(i);
    pv.clear();

    pdv.resize(10000);
    for(unsigned int i = 0; i < pdv.size(); i++)
        pdv.push_back(d);
    pdv.clear();
    t2 = get_clock();
    u1 = get_clock_used_ms(t1, t2);
    fprintf (stderr, "picasso pod_vector use %f ms\n", u1);

    clear_cache();

    agg::pod_vector<unsigned int> av;
    agg::pod_vector<data_test> adv;
    // agg data vector speed.
    t1 = get_clock();

    av.resize(10000);
    for(unsigned int i = 0; i < av.size(); i++)
        av.push_back(i);
    av.clear();

    adv.resize(10000);
    for(unsigned int i = 0; i < adv.size(); i++)
        adv.push_back(d);
    adv.clear();

    t2 = get_clock();
    u2 = get_clock_used_ms(t1, t2);
    fprintf (stderr, "agg pod_vector use %f ms\n", u2);

    clear_cache();

    EXPECT_EQ(true, u1<=u2);
    if (u1 <= u2)
        fprintf (stderr, "picasso is faster!\n");
    else
        fprintf (stderr, "agg is faster!\n");

}


TEST(Pod_BVector, SpeedCompareWithAgg)
{
    clocktime_t t1, t2;
    double u1, u2;

    clear_cache();

    pod_bvector<unsigned int> pv;
    pod_bvector<data_test> pdv;
    data_test d = {1,1,1};

    printf("pod_bvector speed testing...\n");
    // picasso data vector speed.
    t1 = get_clock();

    for(unsigned int i = 0; i < 1000; i++)
        pv.add(i);
    pv.clear();

    for(unsigned int i = 0; i < 1000; i++)
        pdv.add(d);
    pdv.clear();

    t2 = get_clock();
    u1 = get_clock_used_ms(t1, t2);
    fprintf (stderr, "picasso pod_bvector use %f ms\n", u1);

    clear_cache();

    agg::pod_bvector<unsigned int> av;
    agg::pod_bvector<data_test> pav;
    // agg data vector speed.
    t1 = get_clock();

    for(unsigned int i = 0; i < 1000; i++)
        av.add(i);
    av.remove_all();

    for(unsigned int i = 0; i < 1000; i++)
        pav.add(d);
    pav.remove_all();

    t2 = get_clock();
    u2 = get_clock_used_ms(t1, t2);
    fprintf (stderr, "agg pod_bvector use %f ms\n", u2);

    clear_cache();

    EXPECT_EQ(true, u1<=u2);
    if (u1 <= u2)
        fprintf (stderr, "picasso is faster!\n");
    else
        fprintf (stderr, "agg is faster!\n");
}

TEST(Pod_BVector, BlockAndAutoSizeVector)
{
    printf("create bvector size 0, capacity 0\n");
    pod_bvector<unsigned int> iv;
    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(0, (int)iv.capacity());

    printf("add datas\n");
    iv.add(10);
    iv.add(11);
    iv.add(12);

    printf("bvector size 3, capacity 32\n");
    EXPECT_EQ(3, (int)iv.size());
    EXPECT_EQ(32, (int)iv.capacity());

    printf("add datas\n");
    iv.add(13);
    iv.add(14);

    printf("bvector size 5, capacity 32\n");
    EXPECT_EQ(5, (int)iv.size());
    EXPECT_EQ(32, (int)iv.capacity());

    for(unsigned int i = 0; i < iv.size(); i++)
        printf("iv: integer vector[%d] = %d\n",i, iv[i]);

    printf("remove last \n");
    iv.remove_last();

    printf("bvector size 4, capacity 32\n");
    EXPECT_EQ(4, (int)iv.size());
    EXPECT_EQ(32, (int)iv.capacity());
    for(unsigned int i = 0; i < iv.size(); i++)
        printf("iv: integer vector[%d] = %d\n",i, iv[i]);

    printf("remove all \n");
    iv.remove_all();

    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(0, (int)iv.capacity());

}

TEST(Block_Allocater, BlockBaseAllocater)
{
    printf("create a block allocater\n");
    block_allocator alloc(16384-16);

    printf("alloc 100 elements aligment 4\n");
    data_test* ds = (data_test*)alloc.allocate(sizeof(data_test)*256, 4);
    EXPECT_EQ(true, ds!=0);

    printf("block memsize %d \n", alloc.all_mem_used());

    data_test* ss = (data_test*)alloc.allocate(sizeof(data_test), 8); 
    EXPECT_EQ(true, ss!=0);
    printf("block memsize %d \n", alloc.all_mem_used());

    printf("free all memory\n");
    alloc.remove_all();

    printf("block memsize %d \n", alloc.all_mem_used());
}

TEST(Pod_Array, CreateAndInitialize)
{
    pod_array<unsigned int> iv;
    pod_array<unsigned int> sv = iv;

    EXPECT_EQ(0, (int)iv.size());
    EXPECT_EQ(0, (int)sv.size());

    EXPECT_NE(&iv , &sv);

    pod_array<data_test> dv(10);
    EXPECT_EQ(10, (int)dv.size());
}

