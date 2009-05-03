
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>/*block_device_operations*/
#include <linux/genhd.h> /* Structure gendisk */
#include <linux/blkdev.h> /* request_queue*/
#include <linux/hdreg.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/vmalloc.h>

#define KERNEL_SECTOR_SIZE 512  
#define DEVICE_MINORS 16         /* Nombre possible de partition */
#define N_SECTORS 2           /* nombre de secteur pour le block device */
#define HARDSECT_SECTOR_SIZE 512 /* "hardware" sector size of the block device */

MODULE_DESCRIPTION("My kernel module");/*Place une description du module ds le fichier objet*/
MODULE_AUTHOR("Neji Wafa & Dhouib Khalil"); /*Place le nom de l'auteur dans le fichier objet*/
MODULE_LICENSE("$LICENSE$");/*Indique le type de license du module*/

static int  MAJOR=0; /*nombre majeur pour identifier le driver*/
static char *DEVICE_NAME = "mondevice"; /*nom du peripheriqe*/


/* Passage de parametres pour le modules*/
/*Le premier parametre est le nombre majeur pour identifier le driver
Le second parametre est le nom du peripherique (nom du fichier disque)*/
module_param( MAJOR, int, 0);
MODULE_PARM_DESC(MAJOR, "Static major number (none = dynamic)");
module_param(DEVICE_NAME, charp, 0000);
MODULE_PARM_DESC(DEVICE_NAME, "The Name of Device");

static struct request_queue *req_queue; /* La request queue associÈ au pÈripherique */


/* Repr√©sentation interne de mon peripherique */
static struct Mon_Peripherique { 
        unsigned long size;     /* Taille de peripherique en secteurs*/
        spinlock_t lock;        /* Pour l'acces exclusif */
        u8 *data;               /* Le tableau de donnees */
        struct gendisk *gdisk;     /* Representation dans le noyau d'un seul peripherique */ 
}*mon_Peripherique=NULL;


/********************* Ouverture du peripherique **********************/

static int Ouverture_Peripherique(struct inode *inode, struct file *filp) 
{	/*Remplisage de la structure priv√©e qui sera placee dabs file->private_data*/
    mon_Peripherique = inode->i_bdev->bd_disk->private_data;
    filp->private_data = mon_Peripherique; 
    printk (KERN_NOTICE "%s: Open Function\n", DEVICE_NAME);
    return 0; 
}
/******************* Fin Ouverture du p√©riph√©rique **********************/ 


/********************* Fermeture du p√©riph√©rique **********************/

static int Fermeture_Peripherique(struct inode *inode, struct file *filp) 
{ 
    
 mon_Peripherique = inode->i_bdev->bd_disk->private_data;
printk (KERN_NOTICE "%s: Release Function\n", DEVICE_NAME);
    return 0;
} 
/******************* Fin Fermeture du peripherique **********************/ 


/*la mÈthode ioctl sert gÈnÈralement ‡ contrÙler le pÈriphÈrique.
Cette fonction permet de passer des commandes particuliËres au pÈriphÈrique. 
Elle prend en compte une seule commande, permettant de dÈfinir et de donner 
des informations sur l'aspect physique (gÈomÈtrique)  du pÈriphÈrique.
*/
int miniblock_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg) 
{ 
        long size; 
        struct hd_geometry geo;    //geometry cylinders and array of sectors.  
        mon_Peripherique = filp->private_data; 
        switch(cmd) { 
            case HDIO_GETGEO: 
                /* 
                * Get geometry: since we are a virtual device, we have to make 
                * up something plausible. So we claim 16 sectors, four heads, 
                * and calculate the corresponding number of cylinders. We set the 
                * start of data at sector four. 
                */ 
                size = mon_Peripherique->size*(HARDSECT_SECTOR_SIZE/KERNEL_SECTOR_SIZE); 
                geo.cylinders = (size & ~0x3f) >> 6; 
                geo.heads = 4; 
                geo.sectors = 16; 
                geo.start = 4; 
                if (copy_to_user((void *) arg, &geo, sizeof(geo))) 
                        return -EFAULT; 
                return 0; 
    }

    printk (KERN_NOTICE "minibd: IOCTL Function\n");
    return -ENOTTY;
}


/* Les operations relatives au peripherique  */
static struct block_device_operations block_dev_op = {
    .owner          = THIS_MODULE,
    .open           = Ouverture_Peripherique,
    .release        = Fermeture_Peripherique,
    .ioctl          = miniblock_ioctl
};




/* Manipulation des requetes I/O du peripherique */ 
static void  PeripheriqueIO_Transfer(struct Mon_Peripherique *dev,unsigned long sector, unsigned long nsect, char *buffer, int write)
{
    unsigned long offset = sector*KERNEL_SECTOR_SIZE; 
    unsigned long nbytes = nsect*KERNEL_SECTOR_SIZE; 
	

    /* S'assurer qu'on n'effectue pas la copie √† la fin du p√©riph√©rique virtuel */
    if ((offset + nbytes) > dev->size) {
        printk (KERN_NOTICE "%s: Beyond-end write (%ld %ld)\n", DEVICE_NAME, offset, nbytes);
        return;
    }
    
    if (write){ /* Ecriture dans le pÈriphÈrique*/
        memcpy(dev->data + offset, buffer, nbytes);
        printk (KERN_NOTICE "%s: Write with %ld with %ld \n", DEVICE_NAME, offset, nbytes); 
    }
    else{ /* Lecture ‡ partir du pÈriphÈrique*/
        memcpy(buffer, dev->data + offset, nbytes); /* copy to buffer   */ 
        printk (KERN_NOTICE "%s: Read with %ld with %ld \n", DEVICE_NAME, offset, nbytes);
    }



    }


/**** PeripheriqueRW_request Fonction pour la manipulation des requetes R/W du block******/ 
static void  PeripheriqueRW_request(struct request_queue *req_q) 
{
 
struct request *req; 
   // 
    while ((req = elv_next_request(req_q)) != NULL) { 
        if (! blk_fs_request(req)) { 
            printk (KERN_NOTICE "Skip non-CMD request\n"); 
            end_request(req, 0); 
            continue;
        }

	 PeripheriqueIO_Transfer(mon_Peripherique,req->sector,req->current_nr_sectors,req->buffer,rq_data_dir(req));
        end_request(req, 1);
	
    }
	

    }





/*********************** Chargement du driver ***********************************/ 
static int MonDriver_init(void) 
{ int i;

/* ETAPE 1 :Ajout d'un driver au noyau (Enregistrement du pilote ): 
	     Cette etape permettera au peripherique d'etre reconnu par le noyau 
	     On utilise la fonction register_blkdev(unsigned int major,const char* name) */
	     MAJOR = register_blkdev( MAJOR, DEVICE_NAME);
	     if (MAJOR < 0) {
                printk(KERN_WARNING "%s: Unable to obtain major number %d\n", DEVICE_NAME, MAJOR);
                return -EBUSY;
        }

/* Etape 2 : Allocation memoire pour le peripherique:
	    On utilisera la fonction kmalloc(size_t size, int flags)*/
            mon_Peripherique = kmalloc(sizeof(struct Mon_Peripherique), GFP_KERNEL);
	    if (mon_Peripherique == NULL) {
                printk(KERN_WARNING "%s: Error in allocating the device with kmalloc\n", DEVICE_NAME);
                goto out_unregister;
        }



/*  ETAPE 3 : Initialisation du peripherique et allocation des memoires sous-jacentes:
              Cette etape permettera d'assurer la disponibilite du peripherique au systeme */
/*ETAPE 3.1: definition de la plage d'adresse*/ 
              memset(mon_Peripherique, 0, sizeof(struct Mon_Peripherique));
/*  ETAPE 3.2 : */            
              mon_Peripherique->size = N_SECTORS*HARDSECT_SECTOR_SIZE; /*initialisation taille p√©riph√©rique */
              mon_Peripherique->data = vmalloc(mon_Peripherique->size); /*allocation m√©moire pour les donn√©es*/
                      for (i=0; i<mon_Peripherique->size;i++)
	                       	memcpy(mon_Peripherique->data + i,"", 1);
              if (mon_Peripherique->data == NULL) {
                printk(KERN_WARNING "%s: Error in initializing the device: vmalloc Failure\n", DEVICE_NAME);
                kfree(mon_Peripherique);
                goto out_unregister;
        }
              

/*   ETAPE 3.3 : : Associer la m√©thode "P√©riph√©riqueRW_request" avec le p√©riph√©rique:
              (La m√©thode "P√©riph√©riqueRW_request" assure les op√©ration de lecture + ecriture
	      dans le block)
	      On utilise la fonction <blk_init_queue>. 
              Le second argument est un spinlock permettant d'√©viter √† la file de requetes l'acc√®s concurrent */
              spin_lock_init(&mon_Peripherique->lock); 
              req_queue = blk_init_queue( PeripheriqueRW_request, &mon_Peripherique->lock);
              if (req_queue == NULL) {
                printk(KERN_WARNING "%s: error in blk_init_queue\n", DEVICE_NAME);
                goto out_free;
        }

/*  ETAPE optionnel (taille par d√©faut est 512) : 
    Informer le noyau sur la taille du secteur que le p√©riph√©rique peut supporter
     blk_queue_hardsect_size(req_queue, HARDSECT_SECTOR_SIZE); */
 

/*  ETAPE 3.4 : Allocation m√©moire,Initialisation et installation de la structure gendisk */
              /*ETAPE 3.4 .1: Allocation m√©moire pour gendisk*/
              mon_Peripherique->gdisk = alloc_disk(DEVICE_MINORS);
              if (!mon_Peripherique->gdisk) {
                printk(KERN_WARNING "%s: Error in allocating the struct gendisk: alloc_disk\n", DEVICE_NAME);
                goto out_free;
              }
              /*  ETAPE 3.5 : Initialisation de la structure gendisk:*/
        	mon_Peripherique->gdisk->major =  MAJOR; 
        	mon_Peripherique->gdisk->first_minor = 0; 
        	mon_Peripherique->gdisk->fops = &block_dev_op; /*Association des m√©thodes du pilotes*/
		    mon_Peripherique->gdisk->queue = req_queue;
        	mon_Peripherique->gdisk->private_data = mon_Peripherique; 
        	snprintf(mon_Peripherique->gdisk->disk_name,10, "%s", DEVICE_NAME+'0');
        	set_capacity(mon_Peripherique->gdisk, N_SECTORS*(HARDSECT_SECTOR_SIZE/KERNEL_SECTOR_SIZE)); 

/*  ETAPE 3.5 : Installer la structure gendisk
		En invoquant la m√©thode add_disck; le pilote sera pr√™t pour recevoir les requ√™tes
*/              add_disk(mon_Peripherique->gdisk);

return 0;

out_free:
        vfree(mon_Peripherique->data);
        kfree(mon_Peripherique);

out_unregister:
        unregister_blkdev(MAJOR, DEVICE_NAME);
        printk (KERN_NOTICE "%s: Unregister Function\n", DEVICE_NAME);
        return -ENOMEM;


} 
/*********************** FIN Chargement du driver *******************************/ 
 
/*********************** Dechargement du driver ********************************/ 


static void MonDriver_exit(void)
{
        del_gendisk(mon_Peripherique->gdisk); 
        put_disk(mon_Peripherique->gdisk); 

        unregister_blkdev(MAJOR, DEVICE_NAME); 
        blk_cleanup_queue(req_queue);

        vfree(mon_Peripherique->data); 
        kfree(mon_Peripherique); 

        printk(KERN_DEBUG "%s: Exit Function\n", DEVICE_NAME);
}
/*********************** FIN Dechargement du driver ****************************/ 


/*************** Declaration of the init and exit functions ***********************/
module_init(MonDriver_init);
module_exit(MonDriver_exit);


