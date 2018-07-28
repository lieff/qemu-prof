QEMU based profiler
==========

QEMU can dump input instruction blocks and trace what block is executed.
We can parse this information and use it for profiling.

Execution example:

```
$ qemu-aarch64 -d in_asm,exec,nochain minimp3_arm vectors/l3-si_huff.bit 2>log.txt
$ ./qemu-prof log.txt
```
or
```
$ qemu-aarch64 -d in_asm,exec,nochain minimp3_arm vectors/l3-si_huff.bit 2>&1 | ./qemu-prof
```

Result:

```
Total executed instructions: 7533052
mp3d_synth 3227850 42.849%
L3_imdct36 854400 11.342%
L3_dct3_9 739200 9.813%
mp3d_synth_pair 599400 7.957%
mp3d_DCT_II 478350 6.350%
mp3d_synth_granule 381150 5.060%
L3_huffman 263381 3.496%
L3_antialias 225300 2.991%
mp3d_scale_pcm 203704 2.704%
L3_change_sign 120450 1.599%
__memcpy_generic 109254 1.450%
L3_ldexp_q2 58650 0.779%
get_bits 58050 0.771%
```
