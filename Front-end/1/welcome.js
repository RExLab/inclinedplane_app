var setupShepherd = function () {
    
   var  shepherd = new Shepherd.Tour({
        defaults: {
            classes: 'shepherd-element shepherd-open shepherd-theme-arrows',
            showCancelLink: true,
            scrollTo: true
        }
    });  
    // Entrar no tour 
    shepherd.addStep('shep_intro', { 
        text: [lang.intro1],
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.leave,
                classes: 'shepherd-button-secondary',
                 action: function () {
                    shepherd.cancel();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    
    // Tour no meio
    shepherd.addStep('shep_cam', { 
        text: [lang.introcamera],
        attachTo: 'img.cam left',
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    
    
    
  
    shepherd.addStep('shep_slider', { 
        text: [lang.introslider],
        attachTo: '#slider bottom',
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    
    
    shepherd.addStep('shep_intro2', { 
        text: [lang.intro2],
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    shepherd.addStep('shep_send', { 
        text: [lang.introsend],
        attachTo: '#send left',
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    shepherd.addStep('shep_drop', { 
        text: [lang.introdrop],
        attachTo: '#drop right',
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    
    shepherd.addStep('shep_tabela', { 
        text: [lang.introtabela],
        attachTo: 'div.tabela top ',
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    shepherd.addStep('shep_intro3', { 
        text: [lang.intro3],
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    shepherd.addStep('shep_forcay', { 
        text: [lang.introforcay],
        attachTo: 'div.forcay top',
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    shepherd.addStep('shep_forcax', { 
        text: [lang.introforcax],
        attachTo: 'div.forcax top',
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.next,
                action: function () {
                    shepherd.next();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    // Finaliza Tour
    
    shepherd.addStep('shep_buttons', { 
        text: [lang.introalerts],
        attachTo: "#alerts top",
        classes: 'shepherd shepherd-open shepherd-theme-arrows shepherd-transparent-text',
        buttons: [
            {
                text: lang.previous,
                classes: 'shepherd-button-secondary',
                action: function () {
                    shepherd.back();
                }
            }, {
                text: lang.done,
                action: function () {
                    shepherd.cancel();
                },
                classes: 'shepherd-button-example-primary'
            }
        ]
    });
    
    shepherd.on('cancel', function () {

    });
    
    shepherd.on('start', function () {
        
    });
    
    return shepherd;

};

