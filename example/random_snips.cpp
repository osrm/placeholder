///////////////////////////////////////////////////////////////
//  Copyright 2011 John Maddock. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt

#include <nil/crypto3/multiprecision/cpp_int.hpp>
#include <nil/crypto3/multiprecision/cpp_bin_float.hpp>
#include <boost/random.hpp>
#include <boost/scoped_ptr.hpp>
#include <iostream>
#include <iomanip>

void t1() {
    //[random_eg1
    //=#include <nil/crypto3/multiprecision/cpp_int.hpp>
    //=#include <boost/random.hpp>
    //=
    //=int main()
    //={
    using namespace nil::crypto3::multiprecision;
    using namespace boost::random;

    //
    // Declare our random number generator type, the underlying generator
    // is the Mersenne twister mt19937 engine, and we'll generate 256 bit
    // random values, independent_bits_engine will make multiple calls
    // to the underlying engine until we have the requested number of bits:
    //
    typedef independent_bits_engine<mt19937, 256, cpp_int> generator_type;
    generator_type gen;
    //
    // Generate some values:
    //
    std::cout << std::hex << std::showbase;
    for (unsigned i = 0; i < 10; ++i)
        std::cout << gen() << std::endl;
    //
    // Alternatively if we wish to generate random values in a fixed-precision
    // type, then we must use an unsigned type in order to adhere to the
    // conceptual requirements of the generator:
    //
    typedef independent_bits_engine<mt19937, 512, uint512_t> generator512_type;
    generator512_type gen512;
    //
    // Generate some 1024-bit unsigned values:
    //
    std::cout << std::hex << std::showbase;
    for (unsigned i = 0; i < 10; ++i)
        std::cout << gen512() << std::endl;
    //=   return 0;
    //=}
    //]
}

//
// Output from t1() is:
//[random_eg1_out
/*`[pre
0xD091BB5C22AE9EF6E7E1FAEED5C31F792082352CF807B7DFE9D300053895AFE1
0xA1E24BBA4EE4092B18F868638C16A625474BA8C43039CD1A8C006D5FFE2D7810
0xF51F2AE7FF1816E4F702EF59F7BADAFA285954A1B9D09511F878C4B3FB2A0137
0xF508E4AA1C1FE6527C419418CC50AA59CCDF2E5C4C0A1F3B2452A9DC01397D8D
0x6BF88C311CCA797AEA6DA4AEA3C78807CACE1969E0E0D4ADF5A14BAB80F00988
0xA7DE9F4CCC450CBA0924668F5C7DC380D96089C53640AC4CEF1A2E6DAE6D9426
0xADC1965B6613BA46C1FB41C2BD9B0ECDBE3DEDFC7989C8EE6468FD6E6C0DF032
0xA7CD66342C826D8B2BD2E4124D4A2DBEB4BF6FA7CC1A89590826328251097330
0x46E46CB0DF577EC20BD1E364262C556418DDA0C9FE7B45D9D2CE21C9D268409A
0xB1E049E1200BFA47512D6E73C3851EEEF341C0817D973E4808D17554A9E20D28
0xD091BB5C22AE9EF6E7E1FAEED5C31F792082352CF807B7DFE9D300053895AFE1A1E24BBA4EE4092B18F868638C16A625474BA8C43039CD1A8C006D5FFE2D7810
0xF51F2AE7FF1816E4F702EF59F7BADAFA285954A1B9D09511F878C4B3FB2A0137F508E4AA1C1FE6527C419418CC50AA59CCDF2E5C4C0A1F3B2452A9DC01397D8D
0x6BF88C311CCA797AEA6DA4AEA3C78807CACE1969E0E0D4ADF5A14BAB80F00988A7DE9F4CCC450CBA0924668F5C7DC380D96089C53640AC4CEF1A2E6DAE6D9426
0xADC1965B6613BA46C1FB41C2BD9B0ECDBE3DEDFC7989C8EE6468FD6E6C0DF032A7CD66342C826D8B2BD2E4124D4A2DBEB4BF6FA7CC1A89590826328251097330
0x46E46CB0DF577EC20BD1E364262C556418DDA0C9FE7B45D9D2CE21C9D268409AB1E049E1200BFA47512D6E73C3851EEEF341C0817D973E4808D17554A9E20D28
0x70518CE6203AC30361ADD0AB35D0430CC3F8E8920D1C8509CB92388E095436BF2FD6E20868A29AF97D61330B753EC6FC7211EFEA7CD15133A574C4FFCB41F198
0xB598EEF6EBBE7347C1332568CEBA5A7046A99459B4AD9F11AE00FEAA00B8B573A7B480B6B5F0B06C29A0EC27A4DAA0101E76A1C574BE91337F94C950C61F6ED6
0xF5B1C7A192E195F8572384D4E0732C8895D41B68CEE496C3394BBD52048CD47CC05309BED23D2D63414DE9C5D2229F23818666A3F0A8B109B2F6B12769A48341
0xE4123C566C548C8FF5941F6194B993AA8C1651342876763C237CE42EC300D11B263821CA3AEB820241EC0F84CF4AC36DD7393EE6FD0FC06A4118A30A551B54A4
0xD074F86F4CC1C54A3E57A70303774CDAEDE43895379CE62759988939E8490DDC325410E1D9352F6A4047080AF47C081D9DB51A85C765D71F79297527FCCA2773
]
*/
//]

void t2() {
    std::cout << std::dec;
    //[random_eg2
    //=#include <nil/crypto3/multiprecision/cpp_int.hpp>
    //=#include <boost/random.hpp>
    //=
    //=int main()
    //={
    using namespace nil::crypto3::multiprecision;
    using namespace boost::random;

    //
    // Generate integers in a given range using uniform_int,
    // the underlying generator is invoked multiple times
    // to generate enough bits:
    //
    mt19937 mt;
    uniform_int_distribution<cpp_int> ui(-(cpp_int(1) << 256), cpp_int(1) << 256);
    //
    // Generate the numbers:
    //
    for (unsigned i = 0; i < 10; ++i)
        std::cout << ui(mt) << std::endl;

    //=   return 0;
    //=}
    //]
}
//[random_eg2_out
/*`
Program output is

[pre
25593993629538149833210527544371584707508847463356155903670894544241785158492
12721121657520147247744796431842326146296294180809160027132416389225539366745
106034929479008809862776424170460808190085984129117168803272987114325199071833
86048861429530654936263414134573980939351899046345384016090167510299251354700
-23473382144925885755951447143660880642389842563343761080591177733698450031250
76840269649240973945508128641415259490679375154523618053296924666747244530145
21638369166612496703991271955994563624044383325105383029306009417224944272131
18829152205014764576551421737727569993966577957447887116062495161081023584880
101521572847669971701030312596819435590097618913255156117898217707115132658117
-97490271301923067621481012355971422109456300816856752380346627103308328292057
]
*/
//]

void t3() {
    //[random_eg3
    //=#include <nil/crypto3/multiprecision/cpp_bin_float.hpp>
    //=#include <boost/random.hpp>
    //=
    //=int main()
    //={
    using namespace nil::crypto3::multiprecision;
    using namespace boost::random;

    mt19937 gen;
    //
    // Generate the values:
    //
    std::cout << std::setprecision(50);
    for (unsigned i = 0; i < 20; ++i)
        std::cout << generate_canonical<cpp_bin_float_50, std::numeric_limits<cpp_bin_float_50>::digits>(gen)
                  << std::endl;
    //=   return 0;
    //=}
    //]
}

//[random_eg3_out
/*`
Which produces the following output:

[pre
0.96886777112423135248554451482797431507115448261086
0.54722059636785192454525760726084778627750790023546
0.99646132554800874317788284808573062871409279729804
0.98110969177693891782396443737643892769773768718591
0.29702944955795083040856753579705872634075574515969
0.63976335709815275010379796044374742646738557798647
0.79792861516022605265555700991255998690336456180995
0.68135953856026596523755400091345037778580909233387
0.47475868061723477935404326837783394169122045199915
0.30191312687731969398296589840622989141067852863748
0.87242882006730022427155209451091472382531795659709
0.82190326480741096300318873712966555706035846579562
0.49058903962146072778707295967429263659897501512813
0.2102090745190061764133345429475530760261103345204
0.4087311609617603484960794513055502599728804206333
0.79397497154919267900450180642484943996546102712187
0.70577425166871982574205252142383800792823003687121
0.64396095652194035523385641523010248768636064728226
0.5737546665965914620678634509134819579811035412969
0.017773895576552474810236796736785695789752666554273
]
*/
//]

void t4() {
    std::cout << std::endl;
    //[random_eg4
    //=#include <nil/crypto3/multiprecision/cpp_bin_float.hpp>
    //=#include <nil/crypto3/multiprecision/cpp_int.hpp>
    //=#include <boost/random.hpp>
    //=
    //=int main()
    //={
    using namespace nil::crypto3::multiprecision;
    using namespace boost::random;
    //
    // Generate some distruted values:
    //
    uniform_real_distribution<cpp_bin_float_50> ur(-20, 20);
    gamma_distribution<cpp_bin_float_50> gd(20);
    independent_bits_engine<mt19937, std::numeric_limits<cpp_bin_float_50>::digits, cpp_int> gen;
    //
    // Generate some values:
    //
    std::cout << std::setprecision(50);
    for (unsigned i = 0; i < 20; ++i)
        std::cout << ur(gen) << std::endl;
    for (unsigned i = 0; i < 20; ++i)
        std::cout << gd(gen) << std::endl;
    //=   return 0;
    //=}
    //]
}

//[random_eg4_out
/*`
Which produces the following output:

[pre
-18.576837157065858312137736538355805944098004018928
4.5605477000094480453928920098152026546185388161216
-1.7611402252150150370944527411235180945558276280598
-2.471338289511354190492328039842914272146783953149
-7.4131520453411321647183692139916357315276121488316
-9.192739117661751364518299455475684051782402347659
7.0126880787149555595443325648941661436898526919013
2.8554749162054097111723076181877881960039268668423
14.390501287552165467965587841551705310012046701036
-8.9747073123748752412086051960748002945548570524149
-8.1305063133718605220959174700954037986278348616362
9.5496899464463627949564295930962040525540578754312
-15.309681742947663333436391348699943078942921692008
2.0454914298189175280771944784358385982869708951824
-10.069253024538932382193363493367304983742246396276
13.449212808583153116670057807764145176004060370818
-6.0065092542772507561228141992257782449634820245355
15.00971466974838379824678369267201922989930663822
16.158514812070905438581736305533045434508525979205
-2.1531361299576399413547008719541457739794964378093
19.398278792113040046930806838893737245011219380822
12.965216582396067073600685365545292876001524716225
19.561779374349650983983836397553672788578622096947
15.982213641588944604037715576313848977716540941271
23.96044616946856385664151481695038833903083043492
21.054716943622792848187523422423642819628010070375
18.596078774135209530930707331338838805575875990091
19.539530839287848627426769425090194390388333335812
17.176133236359396942946640290935498641489373354297
16.228802394876800099035133760539461530246286999827
23.63807160907473465631049083277558060813997674519
12.838499607321990428122225501321564153572478845401
16.878362445712403300584931374939967549572637230102
20.646246409377134464856282996941395597420615529803
16.602429236226052406561338766554127142762673418695
21.680007865714197450495711030406314524681744024329
21.038948660115771777833205901845639760348321521616
30.494499676527802078320016654058105593076348727966
18.704734464995637480940828829962787676146589788572
22.502216997171061548799304902323434654678156658236
]
*/
//]

void t5() {
    //[random_eg5
    //=#include <nil/crypto3/multiprecision/cpp_bin_float.hpp>
    //=#include <boost/random.hpp>
    //=#include <boost/scoped_ptr.hpp>
    //=
    //=int main()
    //={
    using namespace nil::crypto3::multiprecision;
    using namespace boost::random;
    //
    // Generate some multiprecision values, note that the generator is so large
    // that we have to allocate it on the heap, otherwise we may run out of
    // stack space!  We could avoid this by using a floating point type which
    // allocates it's internal storage on the heap - cpp_bin_float will do
    // this with the correct template parameters, as will the GMP or MPFR
    // based reals.
    //
    typedef lagged_fibonacci_01_engine<cpp_bin_float_50, 48, 44497, 21034> big_fib_gen;
    boost::scoped_ptr<big_fib_gen> pgen(new big_fib_gen);
    //
    // Generate some values:
    //
    std::cout << std::setprecision(50);
    for (unsigned i = 0; i < 20; ++i)
        std::cout << (*pgen)() << std::endl;
    //
    // try again with a ranlux generator, this is not quite so large
    // so we can use the heap this time:
    //
    typedef subtract_with_carry_01_engine<cpp_bin_float_50, std::numeric_limits<cpp_bin_float_50>::digits - 5, 10, 24>
        ranlux_big_base_01;
    typedef discard_block_engine<ranlux_big_base_01, 389, 24> big_ranlux;
    big_ranlux rg;
    for (unsigned i = 0; i < 20; ++i)
        std::cout << rg() << std::endl;
    //=   return 0;
    //=}
    //]
}

int main() {
    t1();
    t2();
    t3();
    t4();
    t5();
    return 0;
}

