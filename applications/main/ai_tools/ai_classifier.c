/**
 * @file ai_classifier.c
 * @brief Implementation of lightweight AI classifier
 */

#include "ai_classifier.h"
#include <string.h>
#include <math.h>

int32_t ai_float_to_fixed(float value) {
    return (int32_t)(value * AI_FIXED_POINT_SCALE);
}

float ai_fixed_to_float(int32_t fixed) {
    return (float)fixed / AI_FIXED_POINT_SCALE;
}

AIDecisionTree* ai_decision_tree_create(uint8_t num_nodes, uint8_t num_classes, uint8_t num_features) {
    if(num_nodes == 0 || num_classes == 0 || num_features == 0) {
        return NULL;
    }

    AIDecisionTree* tree = malloc(sizeof(AIDecisionTree));
    if(!tree) return NULL;

    tree->nodes = malloc(sizeof(AIDecisionNode) * num_nodes);
    if(!tree->nodes) {
        free(tree);
        return NULL;
    }

    tree->num_nodes = num_nodes;
    tree->num_classes = num_classes;
    tree->num_features = num_features;

    // Initialize all nodes
    memset(tree->nodes, 0, sizeof(AIDecisionNode) * num_nodes);

    return tree;
}

void ai_decision_tree_free(AIDecisionTree* tree) {
    if(tree) {
        if(tree->nodes) {
            free(tree->nodes);
        }
        free(tree);
    }
}

AIClassifierResult ai_decision_tree_classify(const AIDecisionTree* tree, const AIFeatureVector* features) {
    AIClassifierResult result = {
        .class_id = 0,
        .confidence = 0,
        .valid = false
    };

    if(!tree || !features || tree->num_nodes == 0) {
        return result;
    }

    if(features->num_features != tree->num_features) {
        return result;
    }

    // Traverse tree starting from root (node 0)
    uint8_t current_node = 0;
    uint8_t depth = 0;
    const uint8_t max_depth = 32; // Prevent infinite loops

    while(depth < max_depth) {
        const AIDecisionNode* node = &tree->nodes[current_node];

        // Check if leaf node
        if(node->feature_idx < 0) {
            result.class_id = node->class_id;
            result.confidence = 100; // Full confidence for decision tree
            result.valid = true;
            break;
        }

        // Check feature bounds
        if(node->feature_idx >= features->num_features) {
            break;
        }

        // Traverse based on threshold
        if(features->features[node->feature_idx] <= node->threshold) {
            current_node = node->left_child;
        } else {
            current_node = node->right_child;
        }

        // Check bounds
        if(current_node >= tree->num_nodes) {
            break;
        }

        depth++;
    }

    return result;
}

AITemplateMatcher* ai_template_matcher_create(uint8_t num_templates, uint8_t num_classes) {
    if(num_templates == 0 || num_classes == 0) {
        return NULL;
    }

    AITemplateMatcher* matcher = malloc(sizeof(AITemplateMatcher));
    if(!matcher) return NULL;

    matcher->templates = malloc(sizeof(AITemplate) * num_templates);
    if(!matcher->templates) {
        free(matcher);
        return NULL;
    }

    matcher->num_templates = num_templates;
    matcher->num_classes = num_classes;

    // Initialize templates
    memset(matcher->templates, 0, sizeof(AITemplate) * num_templates);

    return matcher;
}

void ai_template_matcher_free(AITemplateMatcher* matcher) {
    if(matcher) {
        if(matcher->templates) {
            // Free individual patterns
            for(uint8_t i = 0; i < matcher->num_templates; i++) {
                if(matcher->templates[i].pattern) {
                    free(matcher->templates[i].pattern);
                }
            }
            free(matcher->templates);
        }
        free(matcher);
    }
}

bool ai_template_matcher_add(AITemplateMatcher* matcher, uint8_t template_idx, const int32_t* pattern, uint16_t length, uint8_t class_id) {
    if(!matcher || template_idx >= matcher->num_templates || !pattern || length == 0) {
        return false;
    }

    AITemplate* tmpl = &matcher->templates[template_idx];

    // Free existing pattern if any
    if(tmpl->pattern) {
        free(tmpl->pattern);
    }

    // Allocate and copy pattern
    tmpl->pattern = malloc(sizeof(int32_t) * length);
    if(!tmpl->pattern) {
        return false;
    }

    memcpy(tmpl->pattern, pattern, sizeof(int32_t) * length);
    tmpl->length = length;
    tmpl->class_id = class_id;

    return true;
}

uint32_t ai_pattern_distance(const int32_t* pattern1, const int32_t* pattern2, uint16_t length) {
    uint64_t sum = 0;

    for(uint16_t i = 0; i < length; i++) {
        int64_t diff = (int64_t)pattern1[i] - (int64_t)pattern2[i];
        sum += (uint64_t)(diff * diff);
    }

    // Return sqrt of sum (approximation)
    // Use simple integer square root
    if(sum == 0) return 0;

    uint64_t root = sum;
    uint64_t prev;
    do {
        prev = root;
        root = (root + sum / root) / 2;
    } while(root < prev);

    return (uint32_t)root;
}

uint8_t ai_pattern_correlation(const int32_t* pattern1, const int32_t* pattern2, uint16_t length) {
    if(length == 0) return 0;

    // Compute means
    int64_t sum1 = 0, sum2 = 0;
    for(uint16_t i = 0; i < length; i++) {
        sum1 += pattern1[i];
        sum2 += pattern2[i];
    }
    int32_t mean1 = sum1 / length;
    int32_t mean2 = sum2 / length;

    // Compute correlation coefficient
    int64_t numerator = 0;
    int64_t var1 = 0, var2 = 0;

    for(uint16_t i = 0; i < length; i++) {
        int32_t diff1 = pattern1[i] - mean1;
        int32_t diff2 = pattern2[i] - mean2;
        numerator += (int64_t)diff1 * diff2;
        var1 += (int64_t)diff1 * diff1;
        var2 += (int64_t)diff2 * diff2;
    }

    if(var1 == 0 || var2 == 0) {
        return 0;
    }

    // Compute correlation (-1 to 1, scale to 0-100)
    int64_t denominator = 1;
    // Approximate sqrt using integer math
    uint64_t temp = (uint64_t)var1 * var2;
    if(temp > 0) {
        uint64_t root = temp;
        uint64_t prev;
        do {
            prev = root;
            root = (root + temp / root) / 2;
        } while(root < prev);
        denominator = (int64_t)root;
    }

    int32_t correlation = (int32_t)((numerator * 100) / denominator);
    
    // Clamp to 0-100 (taking absolute value)
    if(correlation < 0) correlation = -correlation;
    if(correlation > 100) correlation = 100;

    return (uint8_t)correlation;
}

AIClassifierResult ai_template_matcher_classify(const AITemplateMatcher* matcher, const int32_t* pattern, uint16_t length) {
    AIClassifierResult result = {
        .class_id = 0,
        .confidence = 0,
        .valid = false
    };

    if(!matcher || !pattern || length == 0 || matcher->num_templates == 0) {
        return result;
    }

    uint32_t best_distance = UINT32_MAX;
    uint8_t best_class = 0;

    // Find closest template
    for(uint8_t i = 0; i < matcher->num_templates; i++) {
        const AITemplate* tmpl = &matcher->templates[i];
        if(!tmpl->pattern || tmpl->length == 0) {
            continue;
        }

        // Use shorter length for comparison
        uint16_t compare_length = (length < tmpl->length) ? length : tmpl->length;

        uint32_t distance = ai_pattern_distance(pattern, tmpl->pattern, compare_length);

        if(distance < best_distance) {
            best_distance = distance;
            best_class = tmpl->class_id;
            result.valid = true;
        }
    }

    if(result.valid) {
        result.class_id = best_class;
        
        // Convert distance to confidence (0-100)
        // Lower distance = higher confidence
        if(best_distance == 0) {
            result.confidence = 100;
        } else if(best_distance < 10000) {
            result.confidence = 100 - (best_distance / 100);
            if(result.confidence > 100) result.confidence = 100;
        } else {
            result.confidence = 10; // Very far, low confidence
        }
    }

    return result;
}
