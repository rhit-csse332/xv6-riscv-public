#include "clone.h"
#include "param.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"

// only growproc for malloc changes

// COW for modifying free + join (wait + if)

// don't rewrite memory api


extern struct proc* allocproc(void);

int clone(int (*fn)(void*), void* stack, int flags, void* arg){
    
    int i, pid;

    struct proc *existing_proc = myproc();  // grab current proc
    struct proc *cloned_proc = allocproc(); // alloc fresh proc
    cloned_proc->pagetable = proc_pagetable(cloned_proc); // generate and assign empty PT with trampoline
    //uint64 trapframe_addr = cloned_proc->trapframe;
    //uint64 trampoline_addr = cloned_proc->trampoline - PGSIZE;
    for(uint64 l2 = 0; l2 < 512; l2++){
        pte_t *pte_existing = &(existing_proc->pagetable)[l2];
        if(*pte_existing & PTE_V){ // There is an entry to copy over
            pte_t *pte_cloned = &(cloned_proc->pagetable)[l2];
            if(!(*pte_cloned & PTE_V)){ // Not the trampoline/trapframe
                *pte_cloned = *pte_existing;
            }else{
                
                pagetable_t cloned_pagetable_d1 = (pagetable_t)PTE2PA(*pte_cloned);
                pagetable_t existing_pagetable_d1 = (pagetable_t)PTE2PA(*pte_existing);
                // CHEAP AND DIRTY LINEARIZED RECURSION DOWN ALL 3 LEVELS
                // TODO - Abstract this walk out into a function which calles recursively
                for(uint64 l1 = 0; l1 < 512; l1++){
                    *pte_existing = &(existing_pagetable_d1)[l1];
                    if(*pte_existing & PTE_V){ // There is an entry to copy over
                        *pte_cloned = &(cloned_pagetable_d1)[l1];
                        if(!(*pte_cloned & PTE_V)){ // Not the trampoline/trapframe
                            pte_cloned = pte_existing;
                        }else{
                            
                            
                            // DIRTY RECURSE TO L0
                            pagetable_t cloned_pagetable_d0 = (pagetable_t)PTE2PA(*pte_cloned);
                            pagetable_t existing_pagetable_d0 = (pagetable_t)PTE2PA(*pte_existing);
                            // CHEAP AND DIRTY LINEARIZED RECURSION DOWN ALL 3 LEVELS
                            // TODO - Abstract this walk out into a function which calles recursively
                            for(uint64 l0 = 0; l0 < 512; l0++){
                                *pte_existing = &(existing_pagetable_d0)[l0];
                                if(*pte_existing & PTE_V){ // There is an entry to copy over
                                    *pte_cloned = &(cloned_pagetable_d0)[l0];
                                    if(!(*pte_cloned & PTE_V)){ // Not the trampoline/trapframe
                                        pte_cloned = pte_existing;
                                    }
                                }
                            }


                        }
                    }
                }


            }
        }
    }

    // TODO - Execution + init proc stuff to run
    

    cloned_proc->sz = existing_proc->sz;

    // copy saved user registers.
    *(cloned_proc->trapframe) = *(existing_proc->trapframe);

    // Cause fork to return 0 in the child.
    cloned_proc->trapframe->a0 = 0;

    // increment reference counts on open file descriptors.
    for(i = 0; i < NOFILE; i++)
        if(existing_proc->ofile[i])
            cloned_proc->ofile[i] = filedup(existing_proc->ofile[i]);
    cloned_proc->cwd = idup(existing_proc->cwd);

    safestrcpy(cloned_proc->name, existing_proc->name, sizeof(existing_proc->name));

    pid = cloned_proc->pid;

    release(&cloned_proc->lock);

    acquire(&wait_lock);
    cloned_proc->parent = existing_proc;
    release(&wait_lock);

    acquire(&cloned_proc->lock);
    cloned_proc->state = RUNNABLE;
    release(&cloned_proc->lock);

    return pid;

}