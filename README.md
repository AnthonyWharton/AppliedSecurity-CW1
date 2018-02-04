# Applied Security Coursework 1

This work was undertaken for the Applied Security unit, whilst undertaking
studies at the University of Bristol. Below is the introduction to the
coursework briefing:

> _RSA is easy enough to implement that everyone should do it._

_- M. Might.  (http://matt.might.net/articles/what-cs-majors-should-know)_

> Efficient implementation of modular multi-precision integer arithmetic (i.e., 
> arithmetic in `Z_n` for some `N`) is a fundamental requirement for many real 
> asymmetric cryptosystems; examples include RSA where `N = p·q` is a product of 
> two primes, and ElGamal and ECC where `N` is itself a prime (so formally we 
> use `F_p`, where `p = N`).

> This coursework focuses on optimised implementation of selected public-key 
> encryption and decryption operations: in some sense, you could think of your 
> solution as a (limited) library for public-key cryptography. The practical 
> nature of this task is important. More specifically, by implementing your 
> solution in C, and using the GNU Multiple Precision Arithmetic Library (GMP) 
> where appropriate, you should a) produce a result that is competitive with 
> _real_ implementations, and thereby obtain a deeper understanding of various 
> topicscovered in theory alone, _plus_ b) develop a starting point for later 
> courseworks  where you are tasked with attacking cryptosystems of this type.

