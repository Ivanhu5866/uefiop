=== OVERVIEW ===

The uefiop is an application that allows users to manipulate UEFI runtime interfaces provided by Bios at runtime.

The source code link:
https://github.com/Ivanhu5866/uefiop.git

=== STATUS === 

Current status: Draft

Current capabilities:
* set and delete uefi variables
* get variable
* set and get wakeup time
* set and get time
* get next variable name
* reset system

Todo
* query variable info
* get next high monotonic count
* query capsule capabilities
* update capsule

=== dependency ===

Uefiop use the kernel module efi-runtime to manipulate the uefi runtime service.
The source code link:
https://github.com/Ivanhu5866/efi-runtime.git


