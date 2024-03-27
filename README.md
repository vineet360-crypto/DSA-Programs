package com.example.workflow;

import org.apache.sling.api.resource.ResourceResolver;
import org.apache.sling.api.resource.ResourceResolverFactory;
import org.apache.sling.api.resource.Resource;
import org.apache.sling.api.resource.ValueMap;
import org.apache.sling.api.resource.ModifiableValueMap;
import org.apache.sling.api.resource.PersistenceException;
import org.osgi.service.component.annotations.Component;
import org.osgi.service.component.annotations.Reference;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import com.day.cq.workflow.WorkflowException;
import com.day.cq.workflow.WorkflowSession;
import com.day.cq.workflow.exec.WorkItem;
import com.day.cq.workflow.exec.WorkflowData;
import com.day.cq.workflow.metadata.MetaDataMap;

@Component(service = WorkflowProcess.class, immediate = true)
public class AssetSizeRestrictionWorkflowStep implements WorkflowProcess {
    
    private static final Logger LOGGER = LoggerFactory.getLogger(AssetSizeRestrictionWorkflowStep.class);
    
    @Reference
    private ResourceResolverFactory resolverFactory;
    
    @Override
    public void execute(WorkItem workItem, WorkflowSession workflowSession, MetaDataMap metaDataMap) throws WorkflowException {
        // Get the uploaded asset
        WorkflowData workflowData = workItem.getWorkflowData();
        String assetPath = workflowData.getPayload().toString();
        
        // Get the resource resolver
        ResourceResolver resolver = null;
        try {
            resolver = resolverFactory.getServiceResourceResolver(null);
            Resource assetResource = resolver.getResource(assetPath);
            ValueMap properties = assetResource.adaptTo(ValueMap.class);
            
            // Check asset size against configured limit
            long maxSize = 30 * 1024 * 1024; // 30MB in bytes
            long assetSize = properties.get("jcr:data/jcr:content/jcr:data", byte[].class).length;
            
            if (assetSize > maxSize) {
                // Asset size exceeds limit, check for admin override
                if (!isAdminOverrideAllowed(workItem)) {
                    // Admin override not allowed, cancel workflow
                    workflowSession.terminateWorkflow(workItem.getWorkflow());
                    LOGGER.info("Asset upload canceled: size exceeds limit and admin override not allowed");
                    return;
                }
            }
        } catch (Exception e) {
            LOGGER.error("Error processing asset upload workflow step", e);
        } finally {
            if (resolver != null && resolver.isLive()) {
                resolver.close();
            }
        }
    }
    
    private boolean isAdminOverrideAllowed(WorkItem workItem) {
        // Check if user is an admin or belongs to admin group
        // You would need to implement your own logic to check user permissions here
        // For demonstration purposes, let's assume admin override is allowed for all users
        return true;
    }
}


import org.apache.sling.api.resource.Resource;
import org.apache.sling.api.resource.ResourceResolver;
import org.apache.sling.api.resource.ResourceResolverFactory;
import org.apache.sling.serviceusermapping.ServiceUserMapped;
import org.osgi.service.component.annotations.Component;
import org.osgi.service.component.annotations.Reference;
import org.osgi.service.component.annotations.ReferenceCardinality;

@Component(service = AdminOverrideChecker.class, immediate = true)
public class AdminOverrideChecker {

    @Reference
    private ResourceResolverFactory resolverFactory;

    @Reference(cardinality = ReferenceCardinality.OPTIONAL)
    private ServiceUserMapped serviceUserMapped;

    public boolean isAdminOverrideAllowed(String userId) {
        ResourceResolver resolver = null;
        try {
            resolver = resolverFactory.getServiceResourceResolver(null);
            if (resolver != null) {
                // Check if the user has admin privileges
                boolean isAdmin = resolver.getUserID().equals(userId) || resolver.isUserAdmin();
                if (isAdmin) {
                    return true;
                }

                // Check if the user belongs to an admin group
                // For demonstration purposes, let's assume an admin group called "administrators"
                Resource adminGroup = resolver.getResource("/home/groups/administrators");
                if (adminGroup != null && resolver.getUserManager().isMember(adminGroup.adaptTo(javax.jcr.Group.class), userId)) {
                    return true;
                }
            }
        } catch (Exception e) {
            // Handle exceptions
        } finally {
            if (resolver != null && resolver.isLive()) {
                resolver.close();
            }
        }
        return false;
    }
}
